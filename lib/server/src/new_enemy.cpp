/**************************************************************************************************
 *  File:       enemy.cpp
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "new_enemy.h"
#include "region.h"
#include "pathfinding.h"
#include "global_state.h"
#include "messaging.h"
#include "util.h"
#include "SFML/System/Sleep.hpp"
#include <iostream>
#include <cassert>
#include <ctime>

using std::cout, std::endl;
using server::global::PlayerList;

namespace server {
namespace {
    constexpr bool DISPLAY_PATHS = false;
    constexpr util::PixelsPerSecond NUDGE_SPEED = 10;
}

Enemy::Enemy(Region* region_ptr, definitions::EntityType enemy_type, sf::Vector2f position) : Enemy{region_ptr, enemy_type, position, position} { }

Enemy::Enemy(Region* region_ptr, definitions::EntityType enemy_type, sf::Vector2f position, sf::Vector2f pack_spawn) :
             region{region_ptr}, spawn_position{pack_spawn}
{
    assert(region_ptr != nullptr);

    static uint16_t identifier = 0;
    data.id = identifier++;
    data.position = position;
    data.type = enemy_type;
    destination = data.position;

    definition = definitions::GetEntityDefinition(enemy_type);
    data.health = definition.base_health;

    animation_tracker = definitions::AnimationTracker::ConstructAnimationTracker(enemy_type);

    current_speed = 0;
    aggro_target = PlayerList[0].Data.id;
    setBehavior(Behavior::None);
    aggression = definition.base_aggression;
    aggro_range = definition.aggro_range;
    flocking_anchor_point = spawn_position;
    current_max_speed = definition.base_movement_speed;
}

void Enemy::Update(sf::Time elapsed)
{
    animation_tracker.Update(elapsed);

    for (auto& [id, timer] : invulnerability_timers)
    {
        timer += elapsed.asSeconds();
    }

    for (auto& [attack_type, attack] : definition.attacks)
    {
        if (attack.has_value())
        {
            attack.value().cooldown_timer += elapsed.asSeconds();
        }
    }

    hopping_cooldown_timer += elapsed.asSeconds();

    if (checkStuck(elapsed))
    {
        return;
    }

    handleAction(elapsed);
}

void Enemy::WeaponHit(uint16_t player_id, uint8_t damage, definitions::WeaponKnockback knockback, sf::Vector2f hit_vector, float invulnerability_window)
{
    if (invulnerability_timers.find(player_id) == invulnerability_timers.end())
    {
        invulnerability_timers[player_id] = 0;
    }
    else if (invulnerability_timers[player_id] < invulnerability_windows[player_id])
    {
        return;
    }
    else if (data.health == 0)
    {
        return;
    }

    if (data.health < damage)
    {
        data.health = 0;
    }
    else
    {
        data.health -= damage;
    }

    if (data.health == 0)
    {
        setAction(Action::None);
        setBehavior(Behavior::Dead);
        changeAnimation("Death");
    }

    invulnerability_timers[player_id] = 0;
    invulnerability_windows[player_id] = invulnerability_window;

    if (knockback.distance > 0)
    {
        knockback_vector = util::Normalize(hit_vector) * (knockback.distance / knockback.duration);
        knockback_distance = knockback.distance;
        knockback_duration = knockback.duration;
        stun_duration = knockback.stun_duration;

        setAction(Action::Knockback);
    }
}

network::EnemyData Enemy::GetData()
{
    return data;
}

Behavior Enemy::GetBehavior()
{
    return current_behavior;
}

Action Enemy::GetAction()
{
    return current_action;
}

const sf::FloatRect Enemy::GetBounds()
{
    // TODO: track origin here somehow
    // TODO: this is actually just a flawed collision dimensions, rip
    sf::Vector2f size = animation_tracker.GetCurrentAnimation().collision_dimensions;
    return sf::FloatRect(data.position.x - size.x / 2, data.position.y - size.y / 2, size.x, size.y);
}

const sf::FloatRect Enemy::GetBounds(sf::Vector2f position)
{
    sf::Vector2f size = animation_tracker.GetCurrentAnimation().collision_dimensions;
    return sf::FloatRect(position.x - size.x / 2, position.y - size.y / 2, size.x, size.y);
}

const sf::Vector2f Enemy::GetPathingSize()
{
    return animation_tracker.GetAnimation("Move").collision_dimensions;
}

int Enemy::GetSiphonRate()
{
    if (feeding_state == FeedingState::Feeding)
    {
        return definition.siphon_rate;
    }
    else
    {
        return 0;
    }
}

void Enemy::setBehavior(Behavior behavior)
{
    wander_state = WanderState::Start;
    feeding_state = FeedingState::Start;
    hunting_state = HuntingState::Start;
    stalking_state = StalkingState::Start;
    leaping_state = LeapingState::Start;
    flocking_state = FlockingState::Start;
    swarming_state = SwarmingState::Start;

    current_behavior = behavior;
}

void Enemy::setAction(Action action)
{
    // reset states
    stunned_state = StunnedState::Start;
    knockback_state = KnockbackState::Start;
    leaping_state = LeapingState::Start;
    tackling_state = TacklingState::Start;
    hopping_state = HoppingState::Start;
    tail_swipe_state = TailSwipeState::Start;

    current_action = action;
}

void Enemy::chooseBehavior()
{
    if (aggroPlayer())
    {
        return;
    }

    if (definition.behaviors[Behavior::Feeding] && (region->Leyline))
    {
        setBehavior(Behavior::Feeding);
        return;
    }

    if (definition.behaviors[Behavior::Wandering])
    {
        setBehavior(Behavior::Wandering);
        return;
    }

    if (definition.behaviors[Behavior::Flocking])
    {
        setBehavior(Behavior::Flocking);
        return;
    }
}

bool Enemy::attack()
{
    std::vector<Action> attacks;
    for (auto& [attack_type, attack] : definition.attacks)
    {
        if (attack.has_value() && attack.value().cooldown_timer >= attack.value().cooldown)
        {
            attacks.push_back(attack_type);
        }
    }

    if (attacks.empty())
    {
        return false;
    }

    std::shuffle(attacks.begin(), attacks.end(), util::RandomGenerator);

    for (auto& attack : attacks)
    {
        auto distance = util::Distance(GetPlayerById(aggro_target, PlayerList).Data.position, data.position);
        if (distance <= definition.attacks[attack].value().range && distance >= definition.attacks[attack].value().minimum_range)
        {
            setAction(attack);
            return true;
        }
    }

    return false;
}

void Enemy::handleAction(sf::Time elapsed)
{
    switch (current_action)
    {
        case Action::Tackling:
        {
            handleTackling(elapsed);
        }
        break;
        case Action::Knockback:
        {
            handleKnockback(elapsed);
        }
        break;
        case Action::Sniffing:
        {

        }
        break;
        case Action::Stunned:
        {
            handleStunned(elapsed);
        }
        break;
        case Action::Leaping:
        {
            handleLeaping(elapsed);
        }
        break;
        case Action::Hopping:
        {
            handleHopping(elapsed);
        }
        break;
        case Action::TailSwipe:
        {
            handleTailSwipe(elapsed);
        }
        break;
        case Action::None:
        {
            handleBehavior(elapsed);
        }
        break;
    }

    nudge(elapsed);
}

void Enemy::handleBehavior(sf::Time elapsed)
{
    switch (current_behavior)
    {
        case Behavior::None:
        {
            chooseBehavior();
        }
        break;
        case Behavior::Wandering:
        {
            handleWandering(elapsed);
        }
        break;
        case Behavior::Feeding:
        {
            handleFeeding(elapsed);
        }
        break;
        case Behavior::Hunting:
        {
            handleHunting(elapsed);
        }
        break;
        case Behavior::Stalking:
        {
            handleStalking(elapsed);
        }
        break;
        case Behavior::Flocking:
        {
            handleFlocking(elapsed);
        }
        break;
        case Behavior::Swarming:
        {
            handleSwarming(elapsed);
        }
        break;
        case Behavior::Dead:
        {
            return;
        }
        break;
    }

    if (is_walking)
    {
        walk(elapsed);
    }
    else
    {
        move(elapsed);
    }
}

void Enemy::move(sf::Time elapsed)
{
    if (data.position == destination || !is_moving)
    {
        return;
    }

    sf::Vector2f goal = getGoal();
    sf::Vector2f steering_force = util::TruncateVector(steer(goal), definition.steering_force * elapsed.asSeconds());
    sf::Vector2f repulsion_force = util::TruncateVector(getRepulsionForce(definition.repulsion_radius), definition.repulsion_force * elapsed.asSeconds());

    float distance = util::Distance(goal, data.position);
    float threshold = animation_tracker.GetCurrentAnimation().collision_dimensions.x;
    if (distance < threshold * 3)
    {
        if (distance <= threshold)
        {
            repulsion_force = sf::Vector2f{0, 0};
        }
        else
        {
            repulsion_force *= (distance - threshold) / (threshold * 2);
        }
    }

    sf::Vector2f velocity = current_velocity + steering_force + repulsion_force;

    if (current_velocity != sf::Vector2f{0, 0})
    {
        util::AngleDegrees angle_between = util::AngleBetween(current_velocity, goal - data.position);
        if ((angle_between > 90 && angle_between < 270) || current_speed > current_max_speed)
        {
            decelerate(elapsed);
        }
        else
        {
            accelerate(elapsed);
        }
    }
    else
    {
        accelerate(elapsed);
    }

    current_velocity = util::TruncateVector(current_velocity + velocity, current_speed);
    sf::Vector2f step = current_velocity * elapsed.asSeconds();

    takeStep(step);
}

void Enemy::walk(sf::Time elapsed)
{
    if (data.position == destination || !is_moving)
    {
        return;
    }

    sf::Vector2f goal = getGoal();
    sf::Vector2f direction = util::Normalize(goal - data.position);
    current_velocity = direction * definition.walking_speed;
    sf::Vector2f step = current_velocity * elapsed.asSeconds();

    takeStep(step);

    if (util::Distance(goal, data.position) <= definition.walking_speed * elapsed.asSeconds())
    {
        data.position = goal;
    }
}

void Enemy::nudge(sf::Time elapsed)
{
    sf::Vector2f aggragate_direction{0, 0};

    for (auto& enemy : region->Enemies)
    {
        if (enemy.data.id == data.id)
        {
            continue;
        }

        if (util::Intersects(GetBounds(), enemy.GetBounds()))
        {
            sf::Vector2f vector = data.position - enemy.data.position;
            aggragate_direction += util::InvertVectorMagnitude(vector, util::Magnitude(vector * 1.2f));
        }
    }

    aggragate_direction = util::Normalize(aggragate_direction);

    sf::Vector2f step = aggragate_direction * NUDGE_SPEED * elapsed.asSeconds();
    takeStep(step);
}

bool Enemy::takeStep(sf::Vector2f step)
{
    sf::FloatRect bounds = GetBounds(data.position + step);
    bool collision = false;

    for (auto& obstacle : region->Obstacles)
    {
        if (!util::Intersects(obstacle, bounds))
        {
            continue;
        }

        collision = true;

        sf::FloatRect slide_bounds = GetBounds(data.position + sf::Vector2f{step.x, 0});
        if (!util::Intersects(obstacle, slide_bounds))
        {
            step.y = 0;
            continue;
        }

        slide_bounds = GetBounds(data.position + sf::Vector2f{0, step.y});
        if (!util::Intersects(obstacle, slide_bounds))
        {
            step.x = 0;
            continue;
        }
    }

    data.position += step;

    return collision;
}

bool Enemy::checkStuck(sf::Time elapsed)
{
    sf::FloatRect bounds = GetBounds(data.position);
    sf::Vector2f direction;

    for (auto& obstacle : region->Obstacles)
    {
        if (util::Intersects(obstacle, bounds))
        {
            sf::Vector2f center;
            center.x = obstacle.left + ((obstacle.left + obstacle.width) / 2);
            center.y = obstacle.top + ((obstacle.top + obstacle.height) / 2);

            direction = util::Normalize(data.position - center);
            data.position += direction * definition.base_movement_speed * 4.0f * elapsed.asSeconds();
            return true;
        }
    }

    return false;
}

sf::Vector2f Enemy::getGoal()
{
    std::vector<sf::FloatRect> obstacles = region->Obstacles;
    util::PathingGraph graph = util::AppendPathingGraph(data.position, destination, region->Obstacles, GetBounds(), region->PathingGraphs[data.type]);
    std::list<sf::Vector2f> path = util::GetPath(graph);

    if (DISPLAY_PATHS)
    {
        //if (data.id == 5)
        {
            for (auto& player : PlayerList)
            {
                network::ServerMessage::DisplayPath(*player.Socket, graph, path);
            }
        }
        sf::sleep(sf::milliseconds(2));
    }

    if (path.empty())
    {
        return data.position;
    }

    return path.front();
}

sf::Vector2f Enemy::steer(sf::Vector2f goal)
{
    sf::Vector2f desired_direction = util::Normalize(goal - data.position);

    return (desired_direction * current_speed) - current_velocity;
}

void Enemy::accelerate(sf::Time elapsed)
{
    if (!is_moving)
    {
        return;
    }

    if (current_speed == current_max_speed)
    {
        return;
    }

    current_speed += definition.acceleration * elapsed.asSeconds();

    if (current_speed > current_max_speed)
    {
        current_speed = current_max_speed;
    }
}

void Enemy::decelerate(sf::Time elapsed)
{
    if (!is_moving)
    {
        return;
    }

    if (current_speed == 0)
    {
        return;
    }

    current_speed -= definition.deceleration * elapsed.asSeconds();

    if (current_speed < 0)
    {
        current_speed = 0;
    }
}

sf::Vector2f Enemy::getRepulsionForce(float distance)
{
    sf::Vector2f aggragate_direction{0, 0};

    for (auto& enemy : region->Enemies)
    {
        if (enemy.data.id == data.id)
        {
            continue;
        }

        if (util::Distance(enemy.GetBounds().getPosition(), data.position) < distance)
        {
            sf::Vector2f vector = data.position - enemy.data.position;
            aggragate_direction += util::InvertVectorMagnitude(vector, distance);
        }
    }

    return aggragate_direction;
}

void Enemy::handleWandering(sf::Time elapsed)
{
    if (previous_behavior != Behavior::Wandering)
    {
        wander_state = WanderState::Start;
    }

    previous_behavior = current_behavior;
    wander_timer += elapsed.asSeconds();

    switch (wander_state)
    {
        case WanderState::Start:
        {
            wander_timer = 0;
            wander_rest_time = util::GetRandomFloat(definition.wander_rest_time_min, definition.wander_rest_time_max);
            wander_state = WanderState::Resting;
            changeAnimation("Rest");
            aggro_range = definition.aggro_range;
            is_moving = false;
            is_walking = true;
            [[fallthrough]];
        }
        case WanderState::Resting:
        {
            if (wander_timer >= wander_rest_time)
            {
                int max_attempts = 10;

                sf::Vector2f new_destination;
                for (int i = 0; i < max_attempts; ++i)
                {
                    if (util::Distance(data.position, spawn_position) < 75)
                    {
                        new_destination = util::GetRandomPositionFromPoint(data.position, 50, 200);
                    }
                    else
                    {
                        new_destination = util::GetRandomPositionInCone(data.position, 50, 200, util::VectorToAngle(spawn_position - data.position), 120);
                    }

                    if (!util::Contains(region->Obstacles, new_destination))
                    {
                        wander_state = WanderState::Moving;
                        changeAnimation("Move");
                        destination = new_destination;
                        is_moving = true;
                        break;
                    }
                }
            }
        }
        break;
        case WanderState::Moving:
        {
            if (data.position == destination)
            {
                wander_state = WanderState::Start;
            }

            aggroPlayer();
        }
        break;
    }
}

void Enemy::handleFeeding(sf::Time elapsed)
{
    (void)elapsed;
    if (previous_behavior != Behavior::Feeding)
    {
        feeding_state = FeedingState::Start;
    }

    previous_behavior = current_behavior;

    switch (feeding_state)
    {
        case FeedingState::Start:
        {
            feeding_state = FeedingState::Moving;
            changeAnimation("Move");
            destination = region->Convoy.Position;
            is_moving = true;
            is_walking = false;
            aggro_range = definition.aggro_range / 2;
            [[fallthrough]];
        }
        case FeedingState::Moving:
        {
            float distance = getConvoyDistance();

            if (aggroPlayer())
            {
                return;
            }

            if (distance <= 300)
            {
                destination = getTargetConvoyPoint();
                feeding_state = FeedingState::Approaching;
            }
        }
        break;
        case FeedingState::Approaching:
        {
            if (util::Distance(destination, data.position) <= definition.feeding_range)
            {
                is_moving = false;
                feeding_state = FeedingState::Feeding;
                changeAnimation("Feed");
            }
        }
        break;
        case FeedingState::Feeding:
        {
            // TODO: this
        }
        break;
    }
}

void Enemy::handleHunting(sf::Time elapsed)
{
    if (previous_behavior != Behavior::Hunting)
    {
        hunting_state = HuntingState::Start;
    }

    previous_behavior = current_behavior;
    hunting_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(aggro_target, PlayerList);

    switch (hunting_state)
    {
        case HuntingState::Start:
        {
            hunting_timer = 0;
            hunting_state = HuntingState::Moving;
            changeAnimation("Move");
            destination = target.Data.position;
            is_moving = true;
            is_walking = false;
            combat_range = definition.combat_range * util::GetRandomFloat(0.9, 1.3);
            [[fallthrough]];
        }
        case HuntingState::Moving:
        {
            destination = target.Data.position;
            float distance = util::Distance(destination, data.position);

            if (distance >= definition.leash_range)
            {
                setBehavior(Behavior::None);
                return;
            }

            if (definition.behaviors[Behavior::Stalking])
            {
                if (distance <= combat_range)
                {
                    setBehavior(Behavior::Stalking);
                    return;
                }
            }
            else
            {
                if (attack())
                {
                    setBehavior(Behavior::None);
                }
            }
        }
        break;
    }
}

void Enemy::handleStalking(sf::Time elapsed)
{
    if (previous_behavior != Behavior::Stalking)
    {
        stalking_state = StalkingState::Start;
    }

    previous_behavior = current_behavior;
    stalking_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(aggro_target, PlayerList);
    float distance = util::Distance(data.position, target.Data.position);

    switch (stalking_state)
    {
        case StalkingState::Start:
        {
            is_moving = false;
            stalking_state = StalkingState::Choosing;
            [[fallthrough]];
        }
        case StalkingState::Choosing:
        {
            stalking_timer = 0;
            if (distance > definition.combat_range * 1.2)
            {
                setBehavior(definitions::Behavior::None);
                return;
            }

            if (util::GetRandomFloat(0, 1) <= aggression && attack())
            {
                stalking_state = StalkingState::RestStart;
                return;
            }

            if (distance <= definition.close_quarters_range && hopping_cooldown_timer >= definition.hopping_cooldown)
            {
                hop_direction = util::Direction::Back;
                setAction(definitions::Action::Hopping);
                stalking_state = StalkingState::RestStart;
                return;
            }

            bool hop = false;
            hop = util::GetRandomFloat(0, 1) > 0.3;

            if (util::GetRandomFloat(0, 1) <= aggression)
            {
                attack();
            }
            else if (hop && hopping_cooldown_timer >= definition.hopping_cooldown)
            {
                float weight = util::GetRandomFloat(0, 1);
                if (weight < 0.5)
                {
                    hop_direction = util::Direction::Left;
                }
                else
                {
                    hop_direction = util::Direction::Right;
                }

                setAction(definitions::Action::Hopping);
            }

            stalking_state = StalkingState::RestStart;
        }
        break;
        case StalkingState::RestStart:
        {
            stalking_timer = 0;
            stalking_rest_time = util::GetRandomFloat(0.5f, 1.0f);
            stalking_state = StalkingState::Resting;
            changeAnimation("Rest");
            [[fallthrough]];
        }
        case StalkingState::Resting:
        {
            if (distance <= definition.close_quarters_range || stalking_timer >= stalking_rest_time)
            {
                stalking_state = StalkingState::Choosing;
            }
        }
        break;
    }
}

void Enemy::handleFlocking(sf::Time elapsed)
{
    (void)elapsed;
    if (previous_behavior != Behavior::Flocking)
    {
        flocking_state = FlockingState::Start;
    }

    previous_behavior = current_behavior;

    switch (flocking_state)
    {
        case FlockingState::Start:
        {
            is_moving = true;
            is_walking = false;
            flocking_state = FlockingState::Flocking;
            changeAnimation("Move");
            [[fallthrough]];
        }
        case FlockingState::Flocking:
        {
            destination = flocking_anchor_point;
            aggroPlayer();
        }
        break;
    }
}

void Enemy::handleSwarming(sf::Time elapsed)
{
    if (previous_behavior != Behavior::Swarming)
    {
        swarming_state = SwarmingState::Start;
    }

    previous_behavior = current_behavior;
    swarming_rest_timer += elapsed.asSeconds();

    const Player& target = GetPlayerById(aggro_target, PlayerList);
    double target_distance = util::Distance(data.position, target.Data.position);

    switch (swarming_state)
    {
        case SwarmingState::Start:
        {
            is_moving = true;
            is_walking = false;
            swarming_state = SwarmingState::Approaching;
            changeAnimation("Move");
            [[fallthrough]];
        }
        case SwarmingState::Approaching:
        {
            destination = target.Data.position;
            if (target_distance <= definition.combat_range)
            {
                setAction(Action::Tackling);
                swarming_state = SwarmingState::Followthrough;
            }
        }
        break;
        case SwarmingState::Followthrough:
        {
            swarming_rest_point = (util::Normalize(current_velocity) * static_cast<float>(definition.combat_range)) + data.position;
            swarming_rest_timer = 0;
            current_max_speed = definition.base_movement_speed;
            swarming_rest_time = util::GetRandomFloat(definition.swarming_rest_time_min, definition.swarming_rest_time_max);
            swarming_state = SwarmingState::Resting;
            [[fallthrough]];
        }
        case SwarmingState::Resting:
        {
            destination = swarming_rest_point;
            if (swarming_rest_timer >= swarming_rest_time)
            {
                setBehavior(Behavior::None);
            }
        }
        break;
    }
}

void Enemy::handleLeaping(sf::Time elapsed)
{
    leaping_timer += elapsed.asSeconds();

    switch (leaping_state)
    {
        case LeapingState::Start:
        {
            leaping_timer = 0;
            leaping_state = LeapingState::Windup;
            changeAnimation("LeapWindup");
            animation_time = animation_tracker.GetAnimationTime("LeapWindup");
            leaping_direction = util::Normalize(GetPlayerById(aggro_target, PlayerList).Data.position - data.position);
            [[fallthrough]];
        }
        case LeapingState::Windup:
        {
            if (leaping_timer >= animation_time)
            {
                leaping_state = LeapingState::Leaping;
                leaping_timer = 0;
                animation_time = animation_tracker.GetAnimationTime("Leap");
            }
        }
        break;
        case LeapingState::Leaping:
        {
            if (leaping_timer >= animation_time)
            {
                definition.attacks[Action::Leaping].value().cooldown_timer = 0;
                leaping_timer = 0;
                leaping_state = LeapingState::Resting;
                changeAnimation("LeapResting");
                animation_time = animation_tracker.GetAnimationTime("LeapResting");
            }

            util::PixelsPerSecond travel_speed = definition.attacks[Action::Leaping].value().travel_distance / animation_time;

            sf::Vector2f step = leaping_direction * travel_speed * elapsed.asSeconds();
            bool collision = takeStep(step);

            // check for a hit
            for (auto& player : PlayerList)
            {
                if (util::Intersects(player.GetBounds(), GetBounds()))
                {
                    player.AddIncomingAttack(definitions::AttackEvent{data.id, definition.attacks[Action::Leaping].value(), data.position});
                    definition.attacks[Action::Leaping].value().cooldown_timer = 0;
                    leaping_timer = 0;
                    leaping_state = LeapingState::Resting;
                    changeAnimation("LeapResting");
                    animation_time = animation_tracker.GetAnimationTime("LeapResting");
                    break;
                }
            }

            if (collision)
            {
                definition.attacks[Action::Leaping].value().cooldown_timer = 0;
                leaping_timer = 0;
                leaping_state = LeapingState::Resting;
                changeAnimation("LeapResting");
                animation_time = animation_tracker.GetAnimationTime("LeapResting");
            }
        }
        break;
        case LeapingState::Resting:
        {
            if (leaping_timer >= animation_time)
            {
                setAction(Action::None);
            }
        }
        break;
    }
}

void Enemy::handleKnockback(sf::Time elapsed)
{
    knockback_timer += elapsed.asSeconds();

    switch (knockback_state)
    {
        case KnockbackState::Start:
        {
            knockback_timer = 0;
            knockback_state = KnockbackState::Knockback;
            changeAnimation("Knockback");
            [[fallthrough]];
        }
        case KnockbackState::Knockback:
        {
            sf::Vector2f step = knockback_vector * elapsed.asSeconds();
            if (takeStep(step) || knockback_timer >= knockback_duration)
            {
                setAction(Action::Stunned);
            }
        }
        break;
    }
}

void Enemy::handleStunned(sf::Time elapsed)
{
    stunned_timer += elapsed.asSeconds();

    switch (stunned_state)
    {
        case StunnedState::Start:
        {
            stunned_timer = 0;
            stunned_state = StunnedState::Stunned;
            [[fallthrough]];
        }
        case StunnedState::Stunned:
        {
            if (stunned_timer >= stun_duration)
            {
                setAction(Action::None);
            }
        }
        break;
    }
}

void Enemy::handleTackling(sf::Time elapsed)
{
    tackle_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(aggro_target, PlayerList);

    switch (tackling_state)
    {
        case TacklingState::Start:
        {
            tackling_state = TacklingState::Tackle;
            current_max_speed = definition.base_movement_speed * 2.5;
            [[fallthrough]];
        }
        case TacklingState::Tackle:
        {
            destination = target.Data.position;
            move(elapsed);

            if (tackle_timer >= definition.attacks[Action::Tackling].value().duration)
            {
                current_max_speed = definition.base_movement_speed;
                tackle_timer = 0;
                setAction(Action::None);
            }

            // check for a hit
            for (auto& player : PlayerList)
            {
                if (util::Intersects(player.GetBounds(), GetBounds()))
                {
                    player.AddIncomingAttack(definitions::AttackEvent{data.id, definition.attacks[Action::Tackling].value(), data.position});
                    definition.attacks[Action::Tackling].value().cooldown_timer = 0;
                    current_max_speed = definition.base_movement_speed;
                    tackle_timer = 0;
                    setAction(Action::None);
                }
            }
        }
        break;
    }
}

void Enemy::handleHopping(sf::Time elapsed)
{
    hopping_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(aggro_target, PlayerList);
    sf::Vector2f target_direction = util::Normalize(target.Data.position - data.position);
    float distance = util::Distance(data.position, target.Data.position);

    switch (hopping_state)
    {
        case HoppingState::Start:
        {
            changeAnimation("HopWindup", hop_direction);
            animation_time = animation_tracker.GetAnimationTime("HopWindup");
            hopping_state = HoppingState::Windup;
            hopping_timer = 0;
            hopping_cooldown_timer = 0;
            [[fallthrough]];
        }
        case HoppingState::Windup:
        {
            if (hopping_timer >= animation_time)
            {
                if (hop_direction == util::Direction::Back)
                {
                    hop_vector = sf::Vector2f{-target_direction.x, -target_direction.y};
                }
                else
                {
                    util::AngleDegrees relative_angle = util::ToDegrees(std::acos((definition.hopping_distance * definition.hopping_distance) / (2 * distance * definition.hopping_distance)));
                    if (hop_direction == util::Direction::Left)
                    {
                        relative_angle = -relative_angle;
                    }

                    util::AngleDegrees target_angle = util::VectorToAngle(target_direction);

                    hop_vector = util::AngleToVector(relative_angle + target_angle);
                }

                hopping_timer = 0;
                hopping_state = HoppingState::Hopping;
                changeAnimation("Hop", hop_direction);
                animation_time = animation_tracker.GetAnimationTime("Hop");
            }
        }
        break;
        case HoppingState::Hopping:
        {
            float speed = definition.hopping_distance / animation_time;
            sf::Vector2f step = hop_vector * speed * elapsed.asSeconds();
            takeStep(step);

            if (hopping_timer >= animation_time)
            {
                setAction(Action::None);
            }
        }
        break;
    }
}

void Enemy::handleTailSwipe(sf::Time elapsed)
{
    tail_swipe_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(aggro_target, PlayerList);
    sf::Vector2f target_direction = util::Normalize(target.Data.position - data.position);

    switch (tail_swipe_state)
    {
        case TailSwipeState::Start:
        {
            tail_swipe_state = TailSwipeState::Swipe;
            tail_swipe_timer = 0;
            changeAnimation("TailSwipe", util::GetOctalDirection(util::VectorToAngle(target_direction)));
            definitions::AnimationVariant variant = definitions::GetAnimationVariant(util::GetOctalDirection(util::VectorToAngle(target_direction)));
            animation_time = animation_tracker.GetAnimationTime(definitions::AnimationIdentifier{"TailSwipe", variant});
            [[fallthrough]];
        }
        case TailSwipeState::Swipe:
        {
            bool damage_frame = false;
            for (unsigned frame : animation_tracker.GetCurrentAnimation().hitbox_frames)
            {
                if (animation_tracker.GetFrame().index == frame)
                {
                    damage_frame = true;
                    break;
                }
            }

            if (damage_frame)
            {
                auto attack_hitboxes = animation_tracker.GetAttackHitboxes();

                for (auto& hitbox : attack_hitboxes)
                {
                    hitbox.left += data.position.x;
                    hitbox.top += data.position.y;

                    // check for a hit
                    for (auto& player : PlayerList)
                    {
                        if (util::Intersects(player.GetBounds(), hitbox))
                        {
                            player.AddIncomingAttack(definitions::AttackEvent{data.id, definition.attacks[Action::TailSwipe].value(), data.position});
                        }
                    }
                }
            }

            if (tail_swipe_timer > animation_time)
            {
                definition.attacks[Action::TailSwipe].value().cooldown_timer = 0;
                setAction(Action::None);
            }
        }
        break;
    }
}

void Enemy::changeAnimation(definitions::AnimationName animation_name)
{
    changeAnimation(animation_name, util::Direction::None);
}

void Enemy::changeAnimation(definitions::AnimationName animation_name, util::Direction direction)
{
    for (auto& player : PlayerList)
    {
        network::ServerMessage::ChangeEnemyAnimation(*player.Socket, data.id, animation_name, direction);
    }

    if (direction == util::Direction::None)
    {
        animation_tracker.SetAnimation(animation_name);
    }
    else
    {
        animation_tracker.SetAnimation(animation_name, definitions::GetAnimationVariant(direction));
    }
}

std::optional<uint16_t> Enemy::playerInRange(float aggro_distance)
{
    bool found = false;
    float lowest_distance = std::numeric_limits<float>::infinity();
    uint16_t index;

    for (auto& player : PlayerList)
    {
        float distance = util::Distance(player.Data.position, data.position);
        if (distance <= aggro_distance && distance < lowest_distance)
        {
            lowest_distance = distance;
            found = true;
            index = player.Data.id;
        }
    }

    if (found)
    {
        return std::optional<uint16_t>(index);
    }
    else
    {
        return std::nullopt;
    }
}

bool Enemy::aggroPlayer()
{
    if (!(definition.behaviors[Behavior::Hunting] || definition.behaviors[Behavior::Swarming]))
    {
        return false;
    }

    auto target = playerInRange(aggro_range);
    if (target.has_value())
    {
        aggro_target = target.value();
        if (definition.behaviors[Behavior::Hunting])
        {
            setBehavior(Behavior::Hunting);
        }
        else if (definition.behaviors[Behavior::Swarming])
        {
            setBehavior(Behavior::Swarming);
        }
        return true;
    }

    return false;
}

sf::Vector2f Enemy::getTargetConvoyPoint()
{
    sf::FloatRect convoy_bounds = region->Convoy.GetBounds();
    sf::FloatRect feeding_zone = convoy_bounds;
    int zone_width = 80;
    feeding_zone.left -= zone_width;
    feeding_zone.top -= zone_width;
    feeding_zone.width += zone_width * 2;
    feeding_zone.height += zone_width * 2;

    if (util::Contains(feeding_zone, data.position))
    {
        return data.position;
    }

    sf::Vector2f inner_point;
    sf::Vector2f outer_point;
    util::IntersectionPoint(convoy_bounds, util::LineVector{data.position, region->Convoy.Position - data.position}, inner_point);
    util::IntersectionPoint(feeding_zone, util::LineVector{data.position, region->Convoy.Position - data.position}, outer_point);

    float min_distance = util::Distance(data.position, outer_point);
    float max_distance = util::Distance(data.position, inner_point);
    util::AngleDegrees angle = util::VectorToAngle(region->Convoy.Position - data.position);

    bool collision;
    sf::Vector2f goal;

    do
    {
        goal = util::GetRandomPositionInCone(data.position, min_distance, max_distance, angle, 120);
    }
    while (!util::Contains(feeding_zone, goal));

    do
    {
        collision = false;
        for (auto& obstacle : region->Obstacles)
        {
            if (util::Intersects(obstacle, GetBounds(goal)))
            {
                collision = true;
                break;
            }
        }

        if (collision)
        {
            goal -= util::Normalize(goal - data.position) * animation_tracker.GetCurrentAnimation().collision_dimensions.x;
        }
    }
    while (collision);

    return goal;
}

float Enemy::getConvoyDistance()
{
    sf::FloatRect convoy_bounds = region->Convoy.GetBounds();

    sf::Vector2f point;
    util::IntersectionPoint(convoy_bounds, util::LineVector{data.position, region->Convoy.Position - data.position}, point);

    return util::Distance(data.position, point);
}

} // server
