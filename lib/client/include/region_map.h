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
#include "definitions.h"
#include "player.h"

namespace client {

class RegionMap
{
public:
    enum class InteractionType
    {
        None, NpcDialog, Shop, ConvoyConsole, ConvoyStash
    };

    struct Interaction
    {
        InteractionType type;
        std::vector<std::string> dialog;
        std::string npc_name;
        uint16_t shop_id;
    };

    struct Npc
    {
        Spritesheet spritesheet;
        std::string name;
        std::vector<std::string> dialog;
        bool fresh_interaction;
        bool shop;
        uint16_t shop_id;
    };

    struct LootItem
    {
        definitions::LootItem definition;
        Spritesheet spritesheet;
    };

    RegionMap();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(definitions::RegionType region);
    void Unload();

    void InitializeRegion(definitions::RegionDefinition definition);
    std::vector<sf::FloatRect> GetInteractablePositions();
    Interaction Interact(sf::Vector2f player_position);
    void SpawnLootItems(std::vector<definitions::LootItem> loot_items);
    void CollectLootItem(definitions::LootItem item);
    bool GetShop(uint16_t shop_id, definitions::Shop& out_shop);
    void UpdateShop(definitions::Shop updated_shop);
    void LeaveRegion();
    void EnterRegion();

    sf::Vector2f GetConvoyPosition();

    definitions::RegionType RegionType;
    sf::FloatRect Bounds;

private:
    Spritesheet background;
    Spritesheet leyline;
    Convoy convoy;
    std::vector<sf::RectangleShape> obstacles;
    std::vector<Npc> npcs;
    std::vector<definitions::Shop> shops;
    std::vector<LootItem> loot_items;

    bool leaving_region = false;
};

} // client
