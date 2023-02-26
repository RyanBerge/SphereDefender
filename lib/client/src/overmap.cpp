/**************************************************************************************************
 *  File:       overmap.cpp
 *  Class:      Overmap
 *
 *  Purpose:    The map that allows players to advance the convoy to different regions
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "overmap.h"
#include "settings.h"
#include "resources.h"
#include "messaging.h"
#include "game_math.h"
#include "game_manager.h"
#include <iostream>

using std::cout, std::endl;
using network::ClientMessage;

namespace client
{

Overmap::Overmap() { }

void Overmap::Load()
{
    marker.LoadAnimationData("gui/interaction_marker.json");

    frame.setSize(Settings::GetInstance().WindowResolution * 0.8f);
    frame.setPosition(Settings::GetInstance().WindowResolution * 0.1f);
    frame.setFillColor(sf::Color{120, 120, 120});
    frame.setOutlineColor(sf::Color::Black);
    frame.setOutlineThickness(10);

    sf::FloatRect frame_bounds = frame.getGlobalBounds();

    information.setPosition(frame_bounds.left + frame_bounds.width / 2, frame_bounds.top + frame_bounds.height * 0.9f);
    information.setFont(*resources::FontManager::GetFont("Vera"));
    information.setCharacterSize(20);
    information.setFillColor(sf::Color::White);

    definitions::Zone zone = definitions::GetZone();

    cancel_button.LoadAnimationData("gui/overmap_cancel.json");
    cancel_button.SetPosition(frame_bounds.left + frame_bounds.width * 0.85f - cancel_button.GetSprite().getGlobalBounds().width, frame_bounds.top + frame_bounds.height * 0.9f);
    cancel_button.RegisterLeftMouseUp([this](void){ ClientMessage::Console(resources::GetServerSocket(), false); });

    confirm_button.LoadAnimationData("gui/overmap_confirm.json");
    confirm_button.SetPosition(frame_bounds.left + frame_bounds.width - confirm_button.GetSprite().getGlobalBounds().width - frame.getOutlineThickness() * 2, frame_bounds.top + frame_bounds.height * 0.9f);
    confirm_button.RegisterOnToggle([this](bool toggled){ castVote(toggled); });
    confirm_button.SetEnabled(false);

    for (auto& id : GameManager::GetInstance().Game.GetPlayerIds())
    {
        vote_indicators[id].indicator = Spritesheet("gui/vote_indicator.json");
        vote_indicators[id].indicator.SetVisible(false);
        vote_indicators[id].vote = -1;
    }

    for (auto& region : zone.regions)
    {
        Node node;
        node.button.SetPosition(frame_bounds.left + frame_bounds.width * region.overmap_position.x / 1000, frame_bounds.top + frame_bounds.height * region.overmap_position.y / 1000);
        node.button.RegisterLeftMouseUp(std::bind(&Overmap::onClickNode, this, region.id));
        node.button.RegisterCursorEnter(std::bind(&Overmap::onHoverNodeEnter, this, region.id));
        node.button.RegisterCursorExit(std::bind(&Overmap::onHoverNodeExit, this));
        switch (region.type)
        {
            case definitions::RegionType::Town:
            {
                node.button.LoadAnimationData("gui/town_overmap_node.json");
            }
            break;
            case definitions::RegionType::Leyline:
            {
                node.button.LoadAnimationData("gui/leyline_overmap_node.json");
            }
            break;
            case definitions::RegionType::Neutral:
            {
                node.button.LoadAnimationData("gui/neutral_overmap_node.json");
            }
            break;
            case definitions::RegionType::Secret:
            {
                node.button.LoadAnimationData("gui/neutral_overmap_node.json");
            }
            break;
        }

        node.definition = region;
        node.visited = false;

        region_nodes.push_back(node);
    }

    for (auto& zone_link : zone.links)
    {
        Link link;
        link.definition = zone_link;

        sf::Vector2f start{region_nodes[link.definition.start].button.GetSprite().getPosition()};
        sf::Vector2f finish{region_nodes[link.definition.finish].button.GetSprite().getPosition()};

        link.line = util::CreateLine(start, finish, sf::Color::Black, 4);
        link.highlight = util::CreateLine(start, finish, sf::Color::Transparent, 4);

        links.push_back(link);
    }

    for (unsigned i = 0; i < region_nodes.size(); ++i)
    {
        util::DjikstraNode node;
        for (auto& link : links)
        {
            if (link.definition.start == i)
            {
                node.connections[link.definition.finish] = link.definition.distance;
            }
            else if (link.definition.finish == i)
            {
                node.connections[link.definition.start] = link.definition.distance;
            }
        }

        node_graph.push_back(node);
    }
}

void Overmap::Update(sf::Time elapsed)
{
    if (active)
    {
        for (auto& node : region_nodes)
        {
            node.button.Update(elapsed);
        }

        for (auto& [id, vote] : vote_indicators)
        {
            vote.indicator.Update(elapsed);
        }

        cancel_button.Update(elapsed);
        confirm_button.Update(elapsed);
    }
}

void Overmap::Draw()
{
    if (active)
    {
        resources::GetWindow().draw(frame);

        for (auto& link : links)
        {
            resources::GetWindow().draw(link.line);
            resources::GetWindow().draw(link.highlight);
        }

        for (auto& node : region_nodes)
        {
            node.button.Draw();
        }

        for (auto& [id, vote] : vote_indicators)
        {
            vote.indicator.Draw();
        }

        cancel_button.Draw();
        confirm_button.Draw();

        marker.Draw();
        resources::GetWindow().draw(information);
    }
}

void Overmap::SetActive(bool is_active, float battery_level)
{
    active = is_active;
    battery = battery_level;
    information.setString("");

    // TODO: There will eventually be ways to look at the overmap without interacting with the console
    if (!is_active)
    {
        confirm_button.SetEnabled(false);
        confirm_button.SetToggled(false);
        for (auto& [id, vote_indicator] : vote_indicators)
        {
            vote_indicator.vote = -1;
            vote_indicator.indicator.SetVisible(false);
        }
    }
}

bool Overmap::IsActive()
{
    return active;
}

void Overmap::SetRegion(uint16_t region_id)
{
    for (auto& node : region_nodes)
    {
        if (node.definition.id == region_id)
        {
            marker.SetPosition(node.button.GetSprite().getPosition().x, node.button.GetSprite().getPosition().y - node.button.GetSprite().getGlobalBounds().height * 0.8f);
            node.button.SetEnabled(false);
            node.visited = true;
            break;
        }
    }

    current_region = region_id;
}

uint16_t Overmap::GetRegion()
{
    return current_region;
}

void Overmap::DisplayVote(uint16_t player_id, uint8_t vote, bool confirmed)
{
    auto& vote_indicator = vote_indicators[player_id];

    if (confirmed)
    {
        vote_indicator.indicator.SetAnimation("Confirmed");
    }
    else
    {
        vote_indicator.indicator.SetAnimation("Unconfirmed");
    }

    sf::FloatRect node_bounds;
    float offset = 0;
    for (auto& [id, other_indicator] : vote_indicators)
    {
        if (player_id != id && other_indicator.indicator.IsVisible() && other_indicator.vote == vote)
        {
            offset += other_indicator.indicator.GetSprite().getGlobalBounds().width * 1.2;
        }
    }

    for (auto& node : region_nodes)
    {
        if (node.definition.id == vote)
        {
            node_bounds = node.button.GetSprite().getGlobalBounds();
            break;
        }
    }

    vote_indicator.indicator.SetPosition(node_bounds.left + offset, node_bounds.top + (node_bounds.height * 1.2));
    vote_indicator.indicator.SetVisible(true);
    vote_indicator.vote = vote;
}

void Overmap::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    for (auto& node : region_nodes)
    {
        node.button.UpdateMousePosition(event);
    }

    cancel_button.UpdateMousePosition(event);
    confirm_button.UpdateMousePosition(event);
}

void Overmap::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    for (auto& node : region_nodes)
    {
        node.button.UpdateMouseState(event, CursorButton::State::Down);
    }

    cancel_button.UpdateMouseState(event, CursorButton::State::Down);
    confirm_button.UpdateMouseState(event, CursorButton::State::Down);
}

void Overmap::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    for (auto& node : region_nodes)
    {
        node.button.UpdateMouseState(event, CursorButton::State::Up);
    }

    cancel_button.UpdateMouseState(event, CursorButton::State::Up);
    confirm_button.UpdateMouseState(event, CursorButton::State::Up);
}

void Overmap::onClickNode(uint16_t region_id)
{
    if (node_graph[current_region].connections.find(region_id) != node_graph[current_region].connections.end())
    {
        if (node_graph[current_region].connections[region_id] > battery)
        {
            information.setString("Not enough battery charge.");
            information.setOrigin(information.getGlobalBounds().width / 2, information.getGlobalBounds().height / 2);
        }
        else
        {
            onHoverNodeExit();
            current_vote = region_id;
            confirm_button.SetToggled(false);
            castVote(confirm_button.GetToggled());
        }
    }
    else
    {
        information.setString("Region is not adjacent.");
        information.setOrigin(information.getGlobalBounds().width / 2, information.getGlobalBounds().height / 2);
    }
}

void Overmap::onHoverNodeEnter(uint16_t node_id)
{
    if (node_id == current_region)
    {
        for (auto& link : links)
        {
            if ((link.definition.start == node_id && !region_nodes[link.definition.finish].visited) ||
                (link.definition.finish == node_id  && !region_nodes[link.definition.start].visited))
            {
                link.highlight.setFillColor(sf::Color::Yellow);
            }
        }
    }
    else
    {
        std::vector<uint16_t> path = util::GetPath(node_graph, current_region, node_id);
        float available_battery = battery;

        for (unsigned i = 1; i < path.size(); ++i)
        {
            uint16_t start = path[i - 1];
            uint16_t finish = path[i];

            for (auto& link : links)
            {
                if (((link.definition.start == start && link.definition.finish == finish) ||
                     (link.definition.finish == start && link.definition.start == finish)) &&
                     !region_nodes[node_id].visited)
                {
                    if (available_battery >= link.definition.distance)
                    {
                        available_battery -= link.definition.distance;
                    }
                    else
                    {
                        if (link.definition.finish == start && link.definition.start == finish)
                        {
                            link.highlight.setOrigin(link.highlight.getGlobalBounds().width, link.highlight.getOrigin().y);
                            link.highlight.setPosition(region_nodes[link.definition.finish].button.GetSprite().getPosition());
                        }

                        link.highlight.setScale(available_battery / link.definition.distance, 1);
                        available_battery = 0;
                    }

                    link.highlight.setFillColor(sf::Color::Yellow);
                }
            }
        }
    }
}

void Overmap::onHoverNodeExit()
{
    for (auto& link : links)
    {
        link.highlight.setOrigin(0, link.highlight.getOrigin().y);
        link.highlight.setPosition(region_nodes[link.definition.start].button.GetSprite().getPosition());
        link.highlight.setFillColor(sf::Color::Transparent);
        link.highlight.setScale(1, 1);
    }
}

void Overmap::castVote(bool toggled)
{
    ClientMessage::CastVote(resources::GetServerSocket(), current_vote, toggled);
    confirm_button.SetEnabled(true);
}

} // namespace client
