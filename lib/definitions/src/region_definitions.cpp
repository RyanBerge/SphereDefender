/**************************************************************************************************
 *  File:       region_definitions.cpp
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region_definitions.h"
#include "game_math.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

using std::cout, std::cerr, std::endl;

namespace definitions
{
    #define REGION_TOWN 0
    #define REGION_LEYLINE 1
    #define REGION_NEUTRAL 2
    #define REGION_SECRET 9
    uint16_t STARTING_REGION = REGION_TOWN;

namespace {

class RegionInitializer
{
public:
    RegionInitializer()
    {
        RegionDefinition region{};

        region.background_file = "backgrounds/cracked_mud.json";
        region.leyline = true;
        region.convoy = ConvoyDefinition(Orientation::North);
        region.convoy.Position = sf::Vector2f{-500, 0};

        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(200, 25, 50, 150)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(100, -100, 50, 150)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(0, 75, 50, 150)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(-125, -300, 100, 250)});

        Regions[RegionType::Leyline] = region;

        region = RegionDefinition{};

        region.background_file = "backgrounds/sand.json";
        region.leyline = false;
        region.convoy = ConvoyDefinition(Orientation::North);
        region.convoy.Position = sf::Vector2f{-500, -200};

        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(-2000, 200, 4000, 600)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(-1500, -1200, 800, 1400)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(500, -1200, 800, 1400)});

        Npc old_man;
        old_man.name = "Old Man";
        old_man.sprite_file = "entities/old_man.json";
        old_man.dialog.push_back("You've come to try one final time? You know my answer has not changed.");
        old_man.dialog.push_back("There is nothing for us out there. Only a fool could believe the Shining City exists. Only a fool could believe they could reach it.");
        old_man.dialog.push_back("They will come for you, you know. In numbers unending. Every time you recharge, they will feel it.");
        old_man.dialog.push_back("Here, we will fade quietly from this world that we have stolen. It is what we deserve. Out there... I have no pity for you.");
        old_man.dialog.push_back("Go. And never return.");
        old_man.position = sf::Vector2f{200, -150};

        region.npcs.push_back(old_man);

        Regions[RegionType::Town] = region;

        region = RegionDefinition{};

        region.background_file = "backgrounds/sand.json";
        region.leyline = false;
        region.convoy = ConvoyDefinition(Orientation::North);
        region.convoy.Position = sf::Vector2f{0, 0};

        Npc david;
        david.name = "David";
        david.sprite_file = "entities/old_man.json";
        david.dialog.push_back("There's nothing here. Come back another day.");
        david.position = sf::Vector2f{400, 0};

        region.npcs.push_back(david);
        Regions[RegionType::Neutral] = region;

        region = RegionDefinition{};

        region.background_file = "backgrounds/cracked_mud.json";
        region.leyline = false;
        region.convoy = ConvoyDefinition(Orientation::North);
        region.convoy.Position = sf::Vector2f{0, 0};

        Npc prophet;
        prophet.name = "The Prophet";
        prophet.sprite_file = "entities/old_man.json";
        prophet.dialog.push_back("How did you get here? What are you doing here?");
        prophet.dialog.push_back("You shouldn't have come.");
        prophet.position = sf::Vector2f{400, 0};

        region.npcs.push_back(prophet);
        Regions[RegionType::Secret] = region;
    }

    std::map<RegionType, RegionDefinition> Regions;
};

}

ConvoyDefinition::ConvoyDefinition() { }

ConvoyDefinition::ConvoyDefinition(Orientation orientation)
{
    load(orientation);
}

sf::FloatRect ConvoyDefinition::GetBounds()
{
    return sf::FloatRect(Position.x - origin.x, Position.y - origin.y, Width, Height);
}

sf::FloatRect ConvoyDefinition::GetInteriorBounds()
{
    sf::Vector2f relative_position{Position.x - origin.x, Position.y - origin.y};
    sf::FloatRect adjusted_interior(interior.left + relative_position.x, interior.top + relative_position.y, interior.width, interior.height);

    return adjusted_interior;
}

std::vector<sf::FloatRect> ConvoyDefinition::GetCollisions()
{
    sf::Vector2f relative_position{Position.x - origin.x, Position.y - origin.y};

    std::vector<sf::FloatRect> adjusted_collisions;

    for (auto& collision : collisions)
    {
        sf::FloatRect adjusted(collision.left + relative_position.x, collision.top + relative_position.y, collision.width, collision.height);
        adjusted_collisions.push_back(adjusted);
    }

    return adjusted_collisions;
}

void ConvoyDefinition::load(Orientation orientation)
{
    std::filesystem::path path;
    if (orientation == Orientation::North || orientation == Orientation::South)
    {
        path = std::filesystem::path("../data/sprites/entities/convoy_vertical.json");
    }
    else
    {
        path = std::filesystem::path("../data/sprites/entities/convoy_horizontal.json");
    }

    if (!std::filesystem::exists(path))
    {
        cerr << "Could not open animation file: " << path << endl;
        return;
    }

    std::ifstream file(path);
    nlohmann::json json;
    file >> json;

    Width = 0;
    Height = 0;

    for (auto& object : json["collisions"])
    {
        sf::FloatRect rect;

        rect.left = object["location"][0];
        rect.top = object["location"][1];
        rect.width = object["size"][0];
        rect.height = object["size"][1];

        if (rect.left + rect.width > Width)
        {
            Width = rect.left + rect.width;
        }

        if (rect.top + rect.height > Height)
        {
            Height = rect.top + rect.height;
        }

        collisions.push_back(rect);
    }

    interior.left = json["interior"]["location"][0];
    interior.top = json["interior"]["location"][1];
    interior.width = json["interior"]["size"][0];
    interior.height = json["interior"]["size"][1];

    origin.x = json["frames"][0]["origin"][0];
    origin.y = json["frames"][0]["origin"][1];
}

RegionDefinition GetRegionDefinition(RegionType region)
{
    static RegionInitializer initializer;
    return initializer.Regions[region];
}

Zone GetZone()
{
    //uint16_t id_counter = 0;

    Zone zone;

    Zone::RegionNode node;
    node.id = 0;
    node.type = RegionType::Town;
    node.overmap_position = sf::Vector2f{100, 900};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 1;
    node.type = RegionType::Leyline;
    node.overmap_position = sf::Vector2f{120, 700};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 2;
    node.type = RegionType::Neutral;
    node.overmap_position = sf::Vector2f{190, 800};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 3;
    node.type = RegionType::Leyline;
    node.overmap_position = sf::Vector2f{280, 760};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 4;
    node.type = RegionType::Neutral;
    node.overmap_position = sf::Vector2f{350, 500};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 5;
    node.type = RegionType::Leyline;
    node.overmap_position = sf::Vector2f{170, 450};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 6;
    node.type = RegionType::Neutral;
    node.overmap_position = sf::Vector2f{320, 820};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 7;
    node.type = RegionType::Neutral;
    node.overmap_position = sf::Vector2f{400, 730};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 8;
    node.type = RegionType::Leyline;
    node.overmap_position = sf::Vector2f{420, 600};
    zone.regions.push_back(node);

    node = Zone::RegionNode{};
    node.id = 9;
    node.type = RegionType::Secret;
    node.overmap_position = sf::Vector2f{650, 500};
    zone.regions.push_back(node);

    Zone::Link link;
    link.start = 0;
    link.finish = 1;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 0;
    link.finish = 2;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 1;
    link.finish = 2;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 2;
    link.finish = 3;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 3;
    link.finish = 4;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 2;
    link.finish = 4;
    link.distance = 400;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 1;
    link.finish = 5;
    link.distance = 550;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 4;
    link.finish = 5;
    link.distance = 200;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 3;
    link.finish = 6;
    link.distance = 75;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 6;
    link.finish = 7;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 7;
    link.finish = 8;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 4;
    link.finish = 8;
    link.distance = 250;
    zone.links.push_back(link);

    link = Zone::Link{};
    link.start = 8;
    link.finish = 9;
    link.distance = 800;
    zone.links.push_back(link);

    return zone;
}

} // definitions
