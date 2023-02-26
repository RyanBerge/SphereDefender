/**************************************************************************************************
 *  File:       entity_definitions.cpp
 *
 *  Purpose:    Information about entities needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "entity_definitions.h"
#include "debug_overrides.h"

namespace definitions {
namespace {

class EntityDefinitionManager
{
public:
    EntityDefinitionManager()
    {
        EntityDefinition small_demon;
        small_demon.base_movement_speed = 105;
        small_demon.size = sf::Vector2f{25, 25};
        small_demon.attack_damage = 30;
        small_demon.attack_range = 36;
        small_demon.minimum_leap_range = 75;
        small_demon.maximum_leap_range = 200;
        small_demon.leaping_speed = 750;
        small_demon.leaping_distance = 225;
        small_demon.feeding_range = 18;
        small_demon.attack_distance = 30;
        small_demon.attack_duration = 0.25;
        small_demon.attack_cooldown = 0.5;

        definition_map[EntityType::SmallDemon] = small_demon;
    }

    std::map<EntityType, EntityDefinition> definition_map;
};

} // anonymous namespace

EntityDefinition GetEntityDefinition(EntityType type)
{
    static EntityDefinitionManager manager;

    return manager.definition_map[type];
}

Weapon GetWeapon(WeaponType type)
{
    Weapon weapon;
    weapon.type = type;

    switch (type)
    {
        case WeaponType::Sword:
        {
            weapon.offset = -18;
            weapon.length = 20;
            weapon.damage = 50;
            weapon.knockback.distance = 35;
            weapon.knockback.duration = 0.1; // seconds
            weapon.attack_cooldown = 250; // milliseconds between attacks
            weapon.arc = 90; // degrees;
            weapon.arc_speed = 360; // degrees per second
            weapon.invulnerability_window = 0.3; // seconds
        }
        break;
        case WeaponType::HitscanGun:
        {
            weapon.offset = -18;
            weapon.length = 7;
            weapon.damage = 50;
            weapon.attack_cooldown = 500; // milliseconds between attacks
            weapon.knockback.distance = 0;
            weapon.knockback.duration = 0;
            weapon.projectiles_per_attack = 1;
            weapon.projectile_spread = 0; // degrees
            weapon.invulnerability_window = 0;
        }
        break;
        case WeaponType::BurstGun:
        {
            weapon.offset = -18;
            weapon.length = 7;
            weapon.damage = 25;
            weapon.attack_cooldown = 900; // milliseconds between attacks
            weapon.knockback.distance = 0;
            weapon.knockback.duration = 0;
            weapon.projectiles_per_attack = 4;
            weapon.delay_per_projectile = 75; // milliseconds
            weapon.projectile_spread = 8; // degrees
            weapon.projectile_speed = 800; // pixels per second
            weapon.invulnerability_window = 0;
        }
        break;
    }

    return weapon;
}

PlayerDefinition::PlayerDefinition()
{
    radius = 18;
    speed = 100;
#ifndef NDEBUG
    if (debug::PlayerMovementSpeed.override)
    {
        speed = debug::PlayerMovementSpeed.value;
    }
#endif
}

PlayerDefinition PlayerDefinition::Get()
{
    static PlayerDefinition definition;
    return definition;
}

} // definitions
