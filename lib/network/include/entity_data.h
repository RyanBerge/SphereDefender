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
#include "entity_definitions.h"
#include <string>

namespace network
{

struct WeaponKnockback
{
    float distance; // total distance over duration in pixels
    int duration; // duration in millisecondss
};

enum class PlayerClass : uint8_t
{
    Melee, Ranged
};

struct PlayerProperties
{
    PlayerClass player_class;
};

struct PlayerData
{
    uint16_t id;
    std::string name;
    sf::Vector2f position;
    uint8_t health;
    PlayerProperties properties;
};

struct EnemyData
{
    uint16_t id;
    shared::EntityType type;
    sf::Vector2f position;
    uint8_t health;
    float charge;
};

} // namespace network
