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

namespace definitions
{

enum class EntityType : uint8_t
{
    Player,
    SmallDemon,
    Bat
};

struct EntityDefinition
{
    sf::Vector2f size;
    float base_movement_speed;
    float attack_damage;
    float attack_range;
    float minimum_leap_range;
    float maximum_leap_range;
    float leaping_speed;
    float leaping_distance;
    float feeding_range;
    float attack_distance;
    float attack_duration;
    float attack_cooldown;
};

EntityDefinition GetEntityDefinition(EntityType type);

enum class WeaponType
{
    Sword, BurstGun, HitscanGun
};

struct WeaponKnockback
{
    float distance; // total distance over duration in pixels
    float duration; // duration in seconds
};

struct Weapon
{
    WeaponType type;
    int damage;
    int attack_cooldown;
    WeaponKnockback knockback;
    int length;
    int offset;
    float invulnerability_window;

    int arc;
    int arc_speed;

    int projectiles_per_attack;
    int delay_per_projectile;
    int projectile_spread;
    int projectile_speed;
};

Weapon GetWeapon(WeaponType type);

struct Projectile
{
    uint16_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool hostile;
    uint16_t owner;
    int damage;
    WeaponKnockback knockback;
    float invulnerability_window;
};

enum class ItemType : uint8_t
{
    None, Medpack
};

struct PlayerDefinition
{
public:
    static PlayerDefinition Get();

    int radius;
    int speed;

private:
    PlayerDefinition();
};

} // definitions
