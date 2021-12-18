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
#include "game_manager.h"
#include "settings.h"
#include "game_math.h"
#include <SFML/System/Time.hpp>
#include <iostream>

using std::cout, std::endl;

namespace client {
namespace {
    constexpr int NPC_DIALOG_DISTANCE = 75;
}

RegionMap::RegionMap()
{

}

void RegionMap::Update(sf::Time elapsed, Player local_player)
{
    if (!leaving_region)
    {
        updateInteractables(local_player.GetPosition());
    }

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
        GameManager::GetInstance().Window.draw(obstacle);
    }

    for (auto& npc : npcs)
    {
        npc.spritesheet.Draw();
    }
}

void RegionMap::Load(shared::RegionName region)
{
    RegionName = region;
    InitializeRegion(shared::GetRegionDefinition(region));
}

void RegionMap::Unload() { }

void RegionMap::InitializeRegion(shared::RegionDefinition definition)
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

    switch (interaction.type)
    {
        case InteractionType::NpcDialog:
        {
            interaction.dialog = npcs[index].dialog;
            interaction.npc_name = npcs[index].name;
            npcs[index].fresh_interaction = false;
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

void RegionMap::updateInteractables(sf::Vector2f player_position)
{
    double distance = std::numeric_limits<double>::infinity();
    unsigned index;

    for (unsigned i = 0; i < npcs.size(); ++i)
    {
        double new_distance = util::Distance(player_position, npcs[i].spritesheet.GetSprite().getPosition());
        if (new_distance < NPC_DIALOG_DISTANCE)
        {
            if (!npcs[i].dialog.empty())
            {
                if (new_distance < distance)
                {
                    distance = new_distance;
                    index = i;
                }
            }
        }

        if (npcs[i].fresh_interaction)
        {
            npcs[i].spritesheet.SetAnimation("AvailableDialog");
        }
        else
        {
            npcs[i].spritesheet.SetAnimation("Default");
        }
    }

    double new_distance = convoy.UpdateInteractables(distance, player_position);

    if (distance < new_distance && distance < std::numeric_limits<double>::infinity())
    {
        if (npcs[index].fresh_interaction)
        {
            npcs[index].spritesheet.SetAnimation("AvailableDialogAndFocus");
        }
        else
        {
            npcs[index].spritesheet.SetAnimation("Focus");
        }
    }
}

sf::Vector2f RegionMap::GetConvoyPosition()
{
    return convoy.GetPosition();
}

} // client
