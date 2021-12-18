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

    constexpr static int WIDTH = 150;
    constexpr static int HEIGHT = 400;
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

struct Npc
{
    std::string name;
    std::string sprite_file;
    std::vector<std::string> dialog;
    sf::Vector2f position;
};

enum class RegionName
{
    Town, Leyline, Neutral
};

struct RegionDefinition
{
    bool leyline;
    std::string background_file;
    ConvoyDefinition convoy;
    std::vector<Obstacle> obstacles;
    std::vector<Npc> npcs;
};

RegionDefinition GetRegionDefinition(RegionName region);
sf::FloatRect GetConvoyBounds(ConvoyDefinition convoy);

} // namespace shared
