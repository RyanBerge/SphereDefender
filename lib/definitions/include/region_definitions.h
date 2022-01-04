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

namespace definitions
{

enum class Orientation
{
    North, South, East, West
};

class ConvoyDefinition
{
public:
    ConvoyDefinition();
    ConvoyDefinition(Orientation orientation);

    sf::FloatRect GetBounds();
    std::vector<sf::FloatRect> GetCollisions();

    sf::Vector2f Position;
    int Width;
    int Height;

    std::vector<sf::FloatRect> collisions;

private:
    sf::Vector2f origin;

    void load(Orientation orientation);
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

enum class RegionType
{
    Town, Leyline, Neutral, Secret
};

struct RegionDefinition
{
    bool leyline;
    std::string background_file;
    ConvoyDefinition convoy;
    std::vector<Obstacle> obstacles;
    std::vector<Npc> npcs;
};

struct Zone
{
    struct RegionNode
    {
        uint16_t id;
        RegionType type;
        sf::Vector2f overmap_position;
    };

    struct Link
    {
        uint16_t start;
        uint16_t finish;
        double distance;
    };

    std::vector<RegionNode> regions;
    std::vector<Link> links;
};

extern uint16_t STARTING_REGION;

RegionDefinition GetRegionDefinition(RegionType region);
Zone GetZone();

} // namespace definitions
