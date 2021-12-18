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
#include "game_manager.h"
#include "region_definitions.h"
#include "messaging.h"

using network::ClientMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client
{

Overmap::Overmap()
{
    frame.setSize(Settings::GetInstance().WindowResolution * 0.8f);
    frame.setPosition(Settings::GetInstance().WindowResolution * 0.1f);
    frame.setFillColor(sf::Color{120, 120, 120});
    frame.setOutlineColor(sf::Color::Black);
    frame.setOutlineThickness(10);

    sf::FloatRect frame_bounds = frame.getGlobalBounds();

    CursorButton town_node("gui/town_overmap_node.json");
    town_node.SetPosition(frame_bounds.left + 100, frame_bounds.top + frame_bounds.height - 100);
    //town_node.RegisterLeftMouseUp([](){ ClientMessage::ChangeRegion(ServerSocket, shared::RegionName::Town); });

    CursorButton leyline_node("gui/leyline_overmap_node.json");
    leyline_node.SetPosition(frame_bounds.left + 100, frame_bounds.top + frame_bounds.height - 300);
    leyline_node.RegisterLeftMouseUp([](){ ClientMessage::ChangeRegion(ServerSocket, shared::RegionName::Leyline); });

    CursorButton neutral_node("gui/neutral_overmap_node.json");
    neutral_node.SetPosition(frame_bounds.left + 200, frame_bounds.top + frame_bounds.height - 140);
    neutral_node.RegisterLeftMouseUp([](){ ClientMessage::ChangeRegion(ServerSocket, shared::RegionName::Neutral); });

    region_nodes.push_back(town_node);
    region_nodes.push_back(leyline_node);
    region_nodes.push_back(neutral_node);
}

void Overmap::Update(sf::Time elapsed)
{
    if (Active)
    {
        for (auto& node : region_nodes)
        {
            node.Update(elapsed);
        }
    }
}

void Overmap::Draw()
{
    if (Active)
    {
        GameManager::GetInstance().Window.draw(frame);
        for (auto& node : region_nodes)
        {
            node.Draw();
        }
    }
}

void Overmap::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    for (auto& node : region_nodes)
    {
        node.UpdateMousePosition(event);
    }
}

void Overmap::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    for (auto& node : region_nodes)
    {
        node.UpdateMouseState(event, CursorButton::State::Down);
    }
}

void Overmap::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    for (auto& node : region_nodes)
    {
        node.UpdateMouseState(event, CursorButton::State::Up);
    }
}


} // namespace client
