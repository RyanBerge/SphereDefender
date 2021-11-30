/**************************************************************************************************
 *  File:       entity_data.h
 *
 *  Purpose:    Data representing entities that goes over the network
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Rect.hpp"
#include <string>

namespace network
{

struct PlayerData
{
    uint16_t id;
    std::string name;
    sf::Vector2f position;
    uint8_t health;
};

struct ConvoyData
{
    enum class Orientation : uint8_t
    {
        //North, Northwest, West, Southwest,
        //South, Southeast, East, Northeast
        North, South, East, West
    };

    sf::Vector2f position;
    Orientation orientation;

    constexpr static int WIDTH = 300;
    constexpr static int HEIGHT = 800;
};

struct EnemyData
{
    enum class EnemyType : uint8_t
    {
        SmallDemon
    };

    uint16_t id;
    EnemyType type;
    sf::Vector2f position;
    uint8_t health;
};


} // namespace network
