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
    sf::FloatRect GetInteriorBounds();
    std::vector<sf::FloatRect> GetCollisions();

    sf::Vector2f Position;
    int Width;
    int Height;

    std::vector<sf::FloatRect> collisions;

private:
    sf::Vector2f origin;
    sf::FloatRect interior;

    void load(Orientation orientation);
};

struct MenuEventOption
{
    uint16_t parent;
    std::string text;
    bool finishing_option;
    uint16_t value;
};

struct MenuEventPage
{
    uint16_t page_index;
    std::string prompt;
    std::vector<MenuEventOption> options;
};

struct MenuEvent
{
    uint16_t event_id;
    std::string name;
    uint16_t current_page;
    std::vector<MenuEventPage> pages;
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

enum class RegionType : uint8_t
{
    Town, Leyline, Neutral, Secret, MenuEvent
};

struct RegionDefinition
{
    bool leyline;
    std::string background_file;
    ConvoyDefinition convoy;
    std::vector<Obstacle> obstacles;
    std::vector<Npc> npcs;
    std::vector<MenuEvent> events;
};

struct Zone
{
    struct RegionNode
    {
        uint16_t id;
        RegionType type;
        sf::Vector2f coordinates;
    };

    struct Link
    {
        uint16_t start;
        uint16_t finish;
        double distance;
    };

    static constexpr int ZONE_WIDTH = 1800;
    static constexpr int ZONE_HEIGHT = 1000;

    std::vector<RegionNode> regions;
    std::vector<Link> links;
};

extern uint16_t STARTING_REGION;

RegionDefinition GetRegionDefinition(RegionType region);
MenuEvent GetNextMenuEvent();
MenuEvent GetMenuEventById(uint16_t id);

} // namespace definitions
