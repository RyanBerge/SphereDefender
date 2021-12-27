/**************************************************************************************************
 *  File:       entity_definitions.cpp
 *
 *  Purpose:    Information about entities needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "entity_definitions.h"

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
        small_demon.maximum_leap_range = 150;
        small_demon.leaping_speed = 750;
        small_demon.leaping_distance = 175;
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

} // definitions
