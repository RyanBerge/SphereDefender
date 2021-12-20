/**************************************************************************************************
 *  File:       region_definitions.cpp
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region_definitions.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

using std::cout, std::cerr, std::endl;

namespace definitions
{

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

        Regions[RegionName::Leyline] = region;

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

        Regions[RegionName::Town] = region;

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
        Regions[RegionName::Neutral] = region;
    }

    std::map<RegionName, RegionDefinition> Regions;
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
    nlohmann::json j;
    file >> j;

    Width = 0;
    Height = 0;

    for (auto& object : j["collisions"])
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

    origin.x = j["frames"][0]["origin"][0];
    origin.y = j["frames"][0]["origin"][1];
}

RegionDefinition GetRegionDefinition(RegionName region)
{
    static RegionInitializer initializer;
    return initializer.Regions[region];
}

} // definitions
