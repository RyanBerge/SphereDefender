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

#include "animation_tracker.h"
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
    void handleAction(sf::Time elapsed);
    void handleBehavior(sf::Time elapsed);

    void handleWandering(sf::Time elapsed);
    void handleFeeding(sf::Time elapsed);
    void handleHunting(sf::Time elapsed);
    void handleStalking(sf::Time elapsed);
    void handleFlocking(sf::Time elapsed);
    void handleSwarming(sf::Time elapsed);

    void handleLeaping(sf::Time elapsed);
    void handleKnockback(sf::Time elapsed);
    void handleStunned(sf::Time elapsed);
    void handleTackling(sf::Time elapsed);
    void handleHopping(sf::Time elapsed);
    void handleTailSwipe(sf::Time elapsed);

    void changeAnimation(network::EnemyAnimation animation);
    void changeAnimation(network::EnemyAnimation animation, util::Direction direction);
    std::optional<uint16_t> playerInRange(float aggro_distance);
    bool aggroPlayer();
    sf::Vector2f getTargetConvoyPoint();
    float getConvoyDistance();

    void move(sf::Time elapsed);
    void walk(sf::Time elapsed);
    bool takeStep(sf::Vector2f step);
    bool checkStuck(sf::Time elapsed);
    sf::Vector2f getGoal();
    sf::Vector2f steer(sf::Vector2f goal);
    void accelerate(sf::Time elapsed);
    void decelerate(sf::Time elapsed);
    sf::Vector2f getRepulsionForce(float distance);

    Region* region = nullptr;
    definitions::EntityDefinition definition;
    network::EnemyData data{};

    Behavior previous_behavior = Behavior::None;
    Action previous_action = Action::None;
    Behavior current_behavior = Behavior::None;
    Action current_action = Action::None;

    definitions::AnimationTracker animation_tracker;
    sf::Vector2f spawn_position;
    sf::Vector2f destination;
    bool is_moving = false;
    bool is_walking = false;
    bool braking = false;
    sf::Vector2f current_velocity{0, 0};
    float current_speed = 0;
    float current_max_speed = 0;
    int aggro_range;
    uint16_t aggro_target;

    std::map<uint16_t, util::Seconds> invulnerability_timers;
    std::map<uint16_t, float> invulnerability_windows;

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
    float aggression;

    enum class StalkingState
    {
        Start,
        Choosing,
        RestStart,
        Resting
    };

    StalkingState stalking_state = StalkingState::Start;
    util::Seconds stalking_timer = 0;
    util::Seconds stalking_rest_time;

    enum class FlockingState
    {
        Start,
        Flocking
    };

    FlockingState flocking_state = FlockingState::Start;
    sf::Vector2f flocking_anchor_point{};

    enum class SwarmingState
    {
        Start,
        Approaching,
        Followthrough,
        Resting
    };

    SwarmingState swarming_state = SwarmingState::Start;
    sf::Vector2f swarming_rest_point{};
    util::Seconds swarming_rest_timer = 0;
    util::Seconds swarming_rest_time = 0;

    enum class KnockbackState
    {
        Start,
        Knockback
    };

    KnockbackState knockback_state = KnockbackState::Start;
    util::Seconds knockback_timer = 0;
    sf::Vector2f knockback_vector{};
    float knockback_distance;
    util::Seconds knockback_duration;

    enum class StunnedState
    {
        Start,
        Stunned
    };

    StunnedState stunned_state = StunnedState::Start;
    util::Seconds stunned_timer = 0;
    util::Seconds stun_duration;

    enum class LeapingState
    {
        Start,
        Windup,
        Leaping,
        Resting
    };

    LeapingState leaping_state = LeapingState::Start;
    util::Seconds leaping_timer = 0;
    sf::Vector2f leaping_direction;

    enum class TacklingState
    {
        Start,
        Tackle,
    };

    TacklingState tackling_state;
    util::Seconds tackle_timer = 0;

    enum class HoppingState
    {
        Start,
        Windup,
        Hopping,
        Resting
    };

    HoppingState hopping_state;
    util::Seconds hopping_timer = 0;
    util::Direction hop_direction;
    sf::Vector2f hop_vector{};

    enum class TailSwipeState
    {
        Start,
        Swipe
    };

    TailSwipeState tail_swipe_state;
    util::Seconds tail_swipe_timer = 0;

};

} // server
