/**************************************************************************************************
 *  File:       region_definitions.cpp
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region_definitions.h"

namespace shared
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
        region.convoy.position = sf::Vector2f{-500, 0};
        region.convoy.orientation = Orientation::North;

        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(200, 25, 50, 150)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(100, -100, 50, 150)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(0, 75, 50, 150)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(-125, -300, 100, 250)});

        Regions[RegionName::Leyline] = region;

        region = RegionDefinition{};

        region.background_file = "backgrounds/sand.json";
        region.leyline = false;
        region.convoy.position = sf::Vector2f{-500, -200};
        region.convoy.orientation = Orientation::North;

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
        region.convoy.position = sf::Vector2f{0, 0};
        region.convoy.orientation = Orientation::North;

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

RegionDefinition GetRegionDefinition(RegionName region)
{
    static RegionInitializer initializer;
    return initializer.Regions[region];
}

sf::FloatRect GetConvoyBounds(ConvoyDefinition convoy)
{
    sf::FloatRect bounds;
    if (convoy.orientation == shared::Orientation::North || convoy.orientation == shared::Orientation::South)
    {
        bounds = sf::FloatRect(convoy.position.x - convoy.WIDTH / 2, convoy.position.y - convoy.HEIGHT / 2, convoy.WIDTH, convoy.HEIGHT);
    }
    else if (convoy.orientation == shared::Orientation::East || convoy.orientation == shared::Orientation::West)
    {
        bounds = sf::FloatRect(convoy.position.x - convoy.HEIGHT / 2, convoy.position.y - convoy.WIDTH / 2, convoy.HEIGHT, convoy.WIDTH);
    }

    return bounds;
}

} // shared
