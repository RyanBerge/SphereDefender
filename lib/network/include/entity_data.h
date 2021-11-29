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
#include <string>

namespace network
{

struct PlayerData
{
    uint16_t id;
    std::string name;
    sf::Vector2f position;
};

enum class EnemyType : uint8_t
{
    SmallDemon
};

struct EnemyData
{
    uint16_t id;
    EnemyType type;
    sf::Vector2f position;
    uint8_t health;
};


} // namespace network
