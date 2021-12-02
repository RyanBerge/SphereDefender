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
        RegionDefinition region;

        region.convoy.position = sf::Vector2f{-600, 0};
        region.convoy.orientation = Orientation::North;

        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(400, 50, 100, 300)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(200, -200, 100, 300)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(0, 50, 100, 300)});
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(-250, -600, 200, 500)});

        Regions["default"] = region;
    }

    std::map<std::string, RegionDefinition> Regions;
};

}

RegionDefinition GetRegionDefinition(std::string region)
{
    static RegionInitializer initializer;
    return initializer.Regions[region];
}

} // shared
