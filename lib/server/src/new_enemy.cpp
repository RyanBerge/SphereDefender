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

constexpr bool DISPLAY_PATHS = false;

namespace server {

Enemy::Enemy(Region* region_ptr) : Enemy{region_ptr, definitions::EntityType::Bat, sf::Vector2f{0, 0}} { }

Enemy::Enemy(Region* region_ptr, definitions::EntityType enemy_type, sf::Vector2f position) : Enemy{region_ptr, enemy_type, position, position} { }

Enemy::Enemy(Region* region_ptr, definitions::EntityType enemy_type, sf::Vector2f position, sf::Vector2f pack_spawn) :
             region{region_ptr}, type{enemy_type}, spawn_position{pack_spawn}
{
    assert(region_ptr != nullptr);

    static uint16_t identifier = 0;
    data.id = identifier++;
    data.position = position;
    destination = data.position;

    definition = definitions::GetEntityDefinition(enemy_type);

    current_max_speed = 0;
    hunting_target = PlayerList[0].Data.id;
    setBehavior(Behavior::None);
    aggression = definition.base_aggression;
    aggro_range = definition.aggro_range;
}

void Enemy::Update(sf::Time elapsed)
{
    switch (current_action)
    {
        case Action::Tackling:
        {

        }
        break;
        case Action::Knockback:
        {

        }
        break;
        case Action::Sniffing:
        {

        }
        break;
        case Action::Stunned:
        {

        }
        break;
        case Action::Leaping:
        {

        }
        break;
        case Action::None:
        {
            handleBehavior(elapsed);
        }
        break;
    }
}

void Enemy::WeaponHit(uint16_t player_id, uint8_t damage, definitions::WeaponKnockback knockback, sf::Vector2f hit_vector, float invulnerability_window)
{
    (void)player_id;
    (void)damage;
    (void)knockback;
    (void)hit_vector;
    (void)invulnerability_window;
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
    sf::Vector2f size = definition.hitbox;
    return sf::FloatRect(data.position.x - size.x / 2, data.position.y - size.y / 2, size.x, size.y);
}

const sf::FloatRect Enemy::GetBounds(sf::Vector2f position)
{
    sf::Vector2f size = definition.hitbox;
    return sf::FloatRect(position.x - size.x / 2, position.y - size.y / 2, size.x, size.y);
}

const sf::FloatRect Enemy::GetProjectedBounds(util::Seconds future)
{
    return GetProjectedBounds(future, current_velocity);
}

const sf::FloatRect Enemy::GetProjectedBounds(util::Seconds future, sf::Vector2f velocity)
{
    sf::Vector2f size = definition.hitbox;
    sf::Vector2f projected_position = data.position + velocity * future;
    return sf::FloatRect(projected_position.x - size.x / 2, projected_position.y - size.y / 2, size.x, size.y);
}

int Enemy::GetSiphonRate()
{
    return 0;
}

void Enemy::setBehavior(Behavior behavior)
{
    current_behavior = behavior;
}

void Enemy::setAction(Action action)
{
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
    }
}

bool Enemy::attack()
{
    return false;
    std::vector<Action> attacks;
    for (auto& [action, available] : definition.actions)
    {
        if (available)
        {
            attacks.push_back(action);
        }
    }

    if (attacks.empty())
    {
        return false;
    }

    srand(std::time(nullptr));
    std::random_shuffle(attacks.begin(), attacks.end());

    setAction(attacks[0]);
    return true;
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
        case Behavior::Dead: { }
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
    float threshold = definition.hitbox.x;
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
        if (angle_between > 90 && angle_between < 270)
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

    current_velocity = util::TruncateVector(current_velocity + velocity, current_max_speed);
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

void Enemy::takeStep(sf::Vector2f step)
{
    sf::FloatRect bounds = GetBounds(data.position + step);
    for (auto& obstacle : region->Obstacles)
    {
        if (!util::Intersects(obstacle, bounds))
        {
            continue;
        }

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
}

sf::Vector2f Enemy::getGoal()
{
    std::vector<sf::FloatRect> obstacles = region->Obstacles;
    util::PathingGraph graph = util::AppendPathingGraph(data.position, destination, region->Obstacles, definition.hitbox, region->PathingGraphs[type]);
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

    return (desired_direction * current_max_speed) - current_velocity;
}

void Enemy::accelerate(sf::Time elapsed)
{
    if (!is_moving)
    {
        return;
    }

    if (current_max_speed == definition.base_movement_speed)
    {
        return;
    }

    current_max_speed += definition.acceleration * elapsed.asSeconds();

    if (current_max_speed > definition.base_movement_speed)
    {
        current_max_speed = definition.base_movement_speed;
    }
}

void Enemy::decelerate(sf::Time elapsed)
{
    if (!is_moving)
    {
        return;
    }

    if (current_max_speed == 0)
    {
        return;
    }

    current_max_speed -= definition.deceleration * elapsed.asSeconds();

    if (current_max_speed < 0)
    {
        current_max_speed = 0;
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
            if (util::Distance(destination, data.position) <= 1)
            {
                is_moving = false;
                feeding_state = FeedingState::Feeding;
            }
        }
        break;
        case FeedingState::Feeding:
        {

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
    stalking_rest_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(hunting_target, PlayerList);

    switch (hunting_state)
    {
        case HuntingState::Start:
        {
            hunting_timer = 0;
            hunting_state = HuntingState::Moving;
            destination = target.Data.position;
            is_moving = true;
            is_walking = false;
            [[fallthrough]];
        }
        case HuntingState::Moving:
        {
            destination = target.Data.position;
            float distance = util::Distance(destination, data.position);
            if (distance <= definition.close_quarters_range && definition.behaviors[Behavior::Stalking] == true)
            {
                setBehavior(Behavior::Stalking);
                return;
            }

            if (distance >= definition.leash_range)
            {
                setBehavior(Behavior::None);
                return;
            }

            if (attack())
            {
                setBehavior(Behavior::None);
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
    stalking_rest_timer += elapsed.asSeconds();
    const Player& target = GetPlayerById(hunting_target, PlayerList);
    float distance = util::Distance(data.position, target.Data.position);

    switch (stalking_state)
    {
        case StalkingState::Start:
        {
            if (distance > definition.close_quarters_range * 1.2f)
            {
                setBehavior(Behavior::None);
                return;
            }

            if (util::GetRandomFloat(0, 1) <= aggression)
            {
                if (attack())
                {
                    return;
                }
            }

            float angle = util::VectorToAngle(data.position - target.Data.position);
            constexpr util::AngleDegrees arc = 60;
            sf::Vector2f goal;
            bool collision = false;

            for (int i = 0; i < 5; ++i)
            {
                goal = util::GetRandomPositionInCone(target.Data.position, definition.close_quarters_range * 0.75f, definition.close_quarters_range, angle, arc);
                collision = false;
                for (auto& obstacle : region->Obstacles)
                {
                    if (util::Contains(obstacle, destination))
                    {
                        collision = true;
                        break;
                    }
                }

                if (!collision)
                {
                    break;
                }
            }

            stalking_state = StalkingState::Moving;

            if (!collision)
            {
                destination = goal;
                stalking_rest_time = util::GetRandomFloat(0.7f, 1.7f);
                is_walking = true;
                is_moving = true;
            }
            else
            {
                attack();
                is_moving = false;
                is_walking = false;
            }
            [[fallthrough]];
        }
        case StalkingState::Moving:
        {
            if (data.position == destination)
            {
                stalking_state = StalkingState::Resting;
                stalking_rest_timer = 0;
                is_moving = false;
            }

            distance = util::Distance(destination, data.position);
            if (distance >= definition.leash_range)
            {
                setBehavior(Behavior::None);
                return;
            }
        }
        break;
        case StalkingState::Resting:
        {
            if (stalking_rest_timer >= stalking_rest_time)
            {
                stalking_state = StalkingState::Start;
            }
        }
        break;
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
    if (!definition.behaviors[Behavior::Hunting])
    {
        return false;
    }

    auto target = playerInRange(aggro_range);
    if (target.has_value())
    {
        hunting_target = target.value();
        setBehavior(Behavior::Hunting);
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
            goal -= util::Normalize(goal - data.position) * definition.hitbox.x;
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
