/**************************************************************************************************
 *  File:       entity_definitions.h
 *
 *  Purpose:    Information about entities needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <map>
#include <cstdint>
#include "SFML/System/Vector2.hpp"

namespace shared
{

enum class EntityType : uint8_t
{
    Player,
    SmallDemon
};

struct EntityDefinition
{
    sf::Vector2f size;
    float base_movement_speed;
    float attack_damage;
    float attack_range;
    float feeding_range;
    float attack_distance;
    float attack_duration;
    float attack_cooldown;
};

EntityDefinition GetEntityDefinition(EntityType type);

namespace PlayerDefinition
{
    constexpr int PLAYER_RADIUS = 18;
    constexpr int PLAYER_SPEED = 100;
    constexpr int SWORD_OFFSET = 18;
    constexpr int SWORD_LENGTH = 20;
    constexpr int GUN_OFFSET = -18;
    constexpr int GUN_LENGTH = 7;
    constexpr int SWORD_DAMAGE = 50;
    constexpr int GUN_DAMAGE = 20;
}

} // shared
