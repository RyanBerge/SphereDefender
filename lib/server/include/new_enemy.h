/**************************************************************************************************
 *  File:       enemy.h
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include "definitions.h"
#include "messaging.h"
#include "game_math.h"

using definitions::Behavior, definitions::Action;

namespace server {

// forward declaration
class Region;

class Enemy
{
public:
    Enemy(Region* region_ptr);
    Enemy(Region* region_ptr, definitions::EntityType enemy_type, sf::Vector2f position);
    Enemy(Region* region_ptr, definitions::EntityType enemy_type, sf::Vector2f position, sf::Vector2f pack_spawn);

    void Update(sf::Time elapsed);
    void WeaponHit(uint16_t player_id, uint8_t damage, definitions::WeaponKnockback knockback, sf::Vector2f hit_vector, float invulnerability_window);

    network::EnemyData GetData();
    Behavior GetBehavior();
    Action GetAction();
    sf::FloatRect GetBounds();
    sf::FloatRect GetBounds(sf::Vector2f position);
    sf::FloatRect GetProjectedBounds(util::Seconds future);
    int GetSiphonRate();

    bool Despawn = false;

private:
    bool chooseAction();
    void handleBehavior(sf::Time elapsed);

    void handleWandering(sf::Time elapsed);
    void handleHunting(sf::Time elapsed);

    void move(sf::Time elapsed);
    void takeStep(sf::Vector2f step);
    sf::Vector2f getGoal();
    bool pathClear(sf::Vector2f direction, util::Seconds time_offset);
    bool checkForwardCollision();
    sf::Vector2f steer(sf::Vector2f goal);
    void accelerate(sf::Time elapsed);
    void decelerate(sf::Time elapsed);
    sf::Vector2f getRepulsionForce(float distance);

    Region* region = nullptr;
    definitions::EntityType type;
    definitions::EntityDefinition definition;
    network::EnemyData data{};

    Behavior previous_behavior = Behavior::Wandering;
    Action previous_action = Action::None;
    Behavior current_behavior = Behavior::Wandering;
    Action current_action = Action::None;

    sf::Vector2f spawn_position;
    sf::Vector2f destination;
    bool is_moving = false;
    bool braking = false;
    sf::Vector2f current_velocity{0, 0};
    float current_max_speed = 0;

    enum class WanderState
    {
        Start,
        Resting,
        Moving
    };

    WanderState wander_state = WanderState::Start;
    util::Seconds wander_timer = 0;
    util::Seconds wander_rest_time = 0;

    enum class HuntingState
    {
        Start,
        Moving,
        Attacking
    };

    HuntingState hunting_state = HuntingState::Start;
    util::Seconds hunting_timer = 0;

};

} // server
