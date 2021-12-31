/**************************************************************************************************
 *  File:       region_map.h
 *  Class:      RegionMap
 *
 *  Purpose:    Represents the world map
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include "convoy.h"
#include "spritesheet.h"
#include "entity_data.h"
#include "region_definitions.h"
#include "player.h"

namespace client {

class RegionMap
{
public:
    enum class InteractionType
    {
        None, NpcDialog, ConvoyConsole, ConvoyStash
    };

    struct Interaction
    {
        InteractionType type;
        std::vector<std::string> dialog;
        std::string npc_name;
    };

    struct Npc
    {
        Spritesheet spritesheet;
        std::string name;
        std::vector<std::string> dialog;
        bool fresh_interaction;
    };

    RegionMap();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(definitions::RegionName region);
    void Unload();

    void InitializeRegion(definitions::RegionDefinition definition);
    std::vector<sf::FloatRect> GetInteractablePositions();
    Interaction Interact(sf::Vector2f player_position);
    void LeaveRegion();
    void EnterRegion();

    sf::Vector2f GetConvoyPosition();

    definitions::RegionName RegionName;

private:
    Spritesheet background;
    Spritesheet leyline;
    Convoy convoy;
    std::vector<sf::RectangleShape> obstacles;
    std::vector<Npc> npcs;

    bool leaving_region = false;
};

} // client
