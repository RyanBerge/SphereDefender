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
        region.obstacles.push_back(Obstacle{ObstacleType::LargeRock, sf::FloatRect(0, 250, 100, 300)});
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
