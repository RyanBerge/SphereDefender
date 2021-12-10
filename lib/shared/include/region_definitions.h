/**************************************************************************************************
 *  File:       region_definitions.h
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <map>
#include <vector>
#include <string>
#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Rect.hpp"

namespace shared
{

enum class Orientation
{
    North, South, East, West
};

struct ConvoyDefinition
{
    sf::Vector2f position;
    Orientation orientation;

    constexpr static int WIDTH = 300;
    constexpr static int HEIGHT = 800;
};

enum class ObstacleType
{
    SmallRock, LargeRock
};

struct Obstacle
{
    ObstacleType type;
    sf::FloatRect bounds;
};

struct RegionDefinition
{
    ConvoyDefinition convoy;

    std::vector<Obstacle> obstacles;
};

RegionDefinition GetRegionDefinition(std::string region);
sf::FloatRect GetConvoyBounds(ConvoyDefinition convoy);

} // namespace shared
