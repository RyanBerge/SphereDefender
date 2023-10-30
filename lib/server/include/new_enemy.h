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
#include <optional>

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
    const sf::FloatRect GetBounds();
    const sf::FloatRect GetBounds(sf::Vector2f position);
    const sf::FloatRect GetProjectedBounds(util::Seconds future);
    const sf::FloatRect GetProjectedBounds(util::Seconds future, sf::Vector2f velocity);
    int GetSiphonRate();

    bool Despawn = false;

private:
    void setBehavior(Behavior behavior);
    void setAction(Action action);
    void chooseBehavior();
    bool attack();
    void handleBehavior(sf::Time elapsed);

    void handleWandering(sf::Time elapsed);
    void handleFeeding(sf::Time elapsed);
    void handleHunting(sf::Time elapsed);
    void handleStalking(sf::Time elapsed);

    void handleLeaping(sf::Time elapsed);

    void changeAnimation(network::EnemyAnimation animation);
    std::optional<uint16_t> playerInRange(float aggro_distance);
    bool aggroPlayer();
    sf::Vector2f getTargetConvoyPoint();
    float getConvoyDistance();

    void move(sf::Time elapsed);
    void walk(sf::Time elapsed);
    void takeStep(sf::Vector2f step);
    bool checkStuck(sf::Time elapsed);
    sf::Vector2f getGoal();
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
    bool is_walking = false;
    bool braking = false;
    sf::Vector2f current_velocity{0, 0};
    float current_max_speed = 0;
    int aggro_range;

    enum class WanderState
    {
        Start,
        Resting,
        Moving
    };

    WanderState wander_state = WanderState::Start;
    util::Seconds wander_timer = 0;
    util::Seconds wander_rest_time = 0;

    enum class FeedingState
    {
        Start,
        Moving,
        Approaching,
        Feeding
    };

    FeedingState feeding_state = FeedingState::Start;

    enum class HuntingState
    {
        Start,
        Moving
    };

    HuntingState hunting_state = HuntingState::Start;
    util::Seconds hunting_timer = 0;
    uint16_t hunting_target;
    float aggression;

    enum class StalkingState
    {
        Start,
        Moving,
        Resting
    };

    StalkingState stalking_state = StalkingState::Start;
    util::Seconds stalking_rest_timer = 0;
    util::Seconds stalking_rest_time;

    enum class LeapingState
    {
        Start,
        Windup,
        Leaping,
        Resting
    };

    LeapingState leaping_state = LeapingState::Start;
    util::Seconds leaping_state_timer = 0;
    sf::Vector2f leaping_direction;

};

} // server
