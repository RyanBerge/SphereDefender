/**************************************************************************************************
 *  File:       entity_definitions.cpp
 *
 *  Purpose:    Information about entities needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "entity_definitions.h"

namespace shared {
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

} // shared
