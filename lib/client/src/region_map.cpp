/**************************************************************************************************
 *  File:       region_map.cpp
 *  Class:      RegionMap
 *
 *  Purpose:    Represents the world map
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region_map.h"
#include "resources.h"
#include "settings.h"
#include "game_math.h"
#include <iostream>

using std::cout, std::endl;

namespace client {
namespace {
    constexpr int NPC_DIALOG_DISTANCE = 75;
}

RegionMap::RegionMap()
{

}

void RegionMap::Update(sf::Time elapsed)
{
    convoy.Update(elapsed);

    leyline.Update(elapsed);

    for (auto& npc : npcs)
    {
        npc.spritesheet.Update(elapsed);
    }
}

void RegionMap::Draw()
{
    background.Draw();
    leyline.Draw();
    convoy.Draw();

    for (auto& obstacle : obstacles)
    {
        resources::GetWindow().draw(obstacle);
    }

    for (auto& npc : npcs)
    {
        npc.spritesheet.Draw();
    }
}

void RegionMap::Load(definitions::RegionType region)
{
    RegionType = region;
    InitializeRegion(definitions::GetRegionDefinition(region));
}

void RegionMap::Unload() { }

void RegionMap::InitializeRegion(definitions::RegionDefinition definition)
{
    background.LoadAnimationData(definition.background_file);
    background.SetTiling(true);
    background.SetPosition(-6000, -6000);

    convoy = Convoy(definition.convoy);

    if (definition.leyline)
    {
        leyline.LoadAnimationData("doodads/leyline.json");
        leyline.SetAnimation("Glow");
        leyline.SetPosition(convoy.GetPosition().x + 120, convoy.GetPosition().y);
    }

    for (auto& obstacle : definition.obstacles)
    {
        sf::RectangleShape rect(obstacle.bounds.getSize());
        rect.setPosition(obstacle.bounds.getPosition());
        rect.setFillColor(sf::Color(100, 100, 100));
        rect.setOutlineColor(sf::Color::Black);
        rect.setOutlineThickness(2);

        obstacles.push_back(rect);
    }

    for (auto& npc_def : definition.npcs)
    {
        Spritesheet spritesheet(npc_def.sprite_file);
        spritesheet.SetPosition(npc_def.position);

        if (npc_def.dialog.size() > 0)
        {
            spritesheet.SetAnimation("AvailableDialog");
        }
        else
        {
            spritesheet.SetAnimation("Default");
        }

        Npc npc;
        npc.spritesheet = spritesheet;
        npc.name = npc_def.name;
        npc.dialog = npc_def.dialog;
        npc.fresh_interaction = true;

        npcs.push_back(npc);
    }
}

std::vector<sf::FloatRect> RegionMap::GetInteractablePositions()
{
    std::vector<sf::FloatRect> bounds_list;

    if (leaving_region)
    {
        return bounds_list;
    }

    for (auto& npc : npcs)
    {
        bounds_list.push_back(npc.spritesheet.GetSprite().getGlobalBounds());
    }

    std::vector<sf::FloatRect> convoy_interactables = convoy.GetInteractablePositions();

    bounds_list.insert(bounds_list.end(), convoy_interactables.begin(), convoy_interactables.end());

    return bounds_list;
}

RegionMap::Interaction RegionMap::Interact(sf::Vector2f player_position)
{
    Interaction interaction;
    interaction.type = InteractionType::None;
    double distance = std::numeric_limits<double>::infinity();
    double new_distance{};
    unsigned index;

    for (unsigned i = 0; i < npcs.size(); ++i)
    {
        new_distance = util::Distance(player_position, npcs[i].spritesheet.GetSprite().getPosition());
        if (new_distance < NPC_DIALOG_DISTANCE)
        {
            if (!npcs[i].dialog.empty() && (interaction.type == InteractionType::None || new_distance < distance))
            {
                interaction.type = InteractionType::NpcDialog;
                distance = new_distance;
                index = i;
            }
        }
    }

    new_distance = util::Distance(convoy.GetConsolePosition(), player_position);
    if (new_distance < distance && new_distance < Convoy::CONSOLE_INTERACTION_DISTANCE)
    {
        distance = new_distance;
        interaction.type = InteractionType::ConvoyConsole;
    }

    new_distance = util::Distance(convoy.GetStashPosition(), player_position);
    if (new_distance < distance && new_distance < Convoy::CONSOLE_INTERACTION_DISTANCE)
    {
        distance = new_distance;
        interaction.type = InteractionType::ConvoyStash;
    }

    switch (interaction.type)
    {
        case InteractionType::NpcDialog:
        {
            interaction.dialog = npcs[index].dialog;
            interaction.npc_name = npcs[index].name;
            npcs[index].fresh_interaction = false;
            npcs[index].spritesheet.SetAnimation("Default");
        }
        break;
        default: { }
    }

    return interaction;
}

void RegionMap::LeaveRegion()
{
    for (auto& npc : npcs)
    {
        npc.spritesheet.SetAnimation("Default");
    }

    convoy.ClearInteractions();
    convoy.LeaveRegion();

    leaving_region = true;
}

void RegionMap::EnterRegion()
{
    convoy.EnterRegion();
}

sf::Vector2f RegionMap::GetConvoyPosition()
{
    return convoy.GetPosition();
}

} // client
