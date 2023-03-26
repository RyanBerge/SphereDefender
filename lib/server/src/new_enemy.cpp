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
#include "SFML/System/Sleep.hpp"
#include <iostream>
#include <cassert>

using std::cout, std::endl;
using server::global::PlayerList;

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

    current_max_speed = definition.base_movement_speed;
    current_behavior = Behavior::Hunting;
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
            if (!chooseAction())
            {
                handleBehavior(elapsed);
            }
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

sf::FloatRect Enemy::GetBounds()
{
    sf::Vector2f size = definition.hitbox;
    return sf::FloatRect(data.position.x - size.x / 2, data.position.y - size.y / 2, size.x, size.y);
}

sf::FloatRect Enemy::GetBounds(sf::Vector2f position)
{
    sf::Vector2f size = definition.hitbox;
    return sf::FloatRect(position.x - size.x / 2, position.y - size.y / 2, size.x, size.y);
}

sf::FloatRect Enemy::GetProjectedBounds(util::Seconds future)
{
    return GetProjectedBounds(future, current_velocity);
}

sf::FloatRect Enemy::GetProjectedBounds(util::Seconds future, sf::Vector2f velocity)
{
    sf::Vector2f size = definition.hitbox;
    sf::Vector2f projected_position = data.position + velocity * future;
    return sf::FloatRect(projected_position.x - size.x / 2, projected_position.y - size.y / 2, size.x, size.y);
}

int Enemy::GetSiphonRate()
{
    return 0;
}

bool Enemy::chooseAction()
{
    return false;
}

void Enemy::handleBehavior(sf::Time elapsed)
{
    switch (current_behavior)
    {
        case Behavior::Wandering:
        {
            handleWandering(elapsed);
        }
        break;
        case Behavior::Moving: { }
        break;
        case Behavior::Feeding: { }
        break;
        case Behavior::Hunting:
        {
            handleHunting(elapsed);
        }
        break;
        case Behavior::Dead: { }
        break;
    }

    move(elapsed);
}

void Enemy::move(sf::Time elapsed)
{
    if (data.position != destination && is_moving)
    {
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
    util::PathingGraph graph = util::CreatePathingGraph(data.position, destination, obstacles, GetBounds().getSize());
    std::list<sf::Vector2f> path = util::GetPath(graph);

//    if (data.id == 1)
//    {
//        for (auto& player : PlayerList)
//        {
//            network::ServerMessage::DisplayPath(*player.Socket, graph, path);
//        }
//    }

    return path.front();
}

sf::Vector2f Enemy::steer(sf::Vector2f goal)
{
    sf::Vector2f desired_direction = util::Normalize(goal - data.position);

    return (desired_direction * current_max_speed) - current_velocity;
}

sf::Vector2f Enemy::avoid(sf::Vector2f direction)
{
    sf::Vector2f desired_direction = util::Normalize(direction - data.position);

    util::VectorCloud cloud = util::CreateVectorCloud(desired_direction);
    cloud = configureDirectionWeights(cloud);
    sf::Vector2f new_direction = util::CollapseVectorCloud(cloud);

    if (new_direction != desired_direction)
    {
        return (desired_direction * current_max_speed) - current_velocity;
    }
    else
    {
        return sf::Vector2f{0, 0};
    }
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

bool Enemy::pathClear(sf::Vector2f direction, util::Seconds time_offset)
{
    float distance = time_offset * definition.base_movement_speed;
    util::LineSegment middle{data.position, data.position + (direction * distance)};
    sf::Vector2f left_translation{-direction.y, direction.x};
    sf::Vector2f right_translation{direction.y, -direction.x};
    float path_width = std::max(definition.hitbox.x, definition.hitbox.y) / 2;
    util::LineSegment left{middle.p1 + (left_translation * path_width), middle.p2 + (left_translation * path_width)};
    util::LineSegment right{middle.p1 + (right_translation * path_width), middle.p2 + (right_translation * path_width)};

    for (auto& enemy : region->Enemies)
    {
        if (data.id == enemy.data.id)
        {
            continue;
        }

        if (util::Intersects(enemy.GetBounds(), middle) ||
            util::Intersects(enemy.GetBounds(), left) ||
            util::Intersects(enemy.GetBounds(), right))
        {
            return false;
        }
    }

    return true;
}

util::VectorCloud Enemy::configureDirectionWeights(util::VectorCloud cloud)
{
    for (auto& vector : cloud)
    {
        sf::FloatRect projected_bounds = GetProjectedBounds(current_max_speed / definition.deceleration, vector.vector * current_max_speed);
        for (auto& obstacle : region->Obstacles)
        {
            if (util::Intersects(projected_bounds, obstacle))
            {
                vector.weight = 0;
                break;
            }
        }
    }

    return cloud;
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
            is_moving = false;
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

    switch (hunting_state)
    {
        case HuntingState::Start:
        {
            hunting_timer = 0;
            hunting_state = HuntingState::Moving;
            destination = PlayerList[0].Data.position;
            is_moving = true;
            [[fallthrough]];
        }
        case HuntingState::Moving:
        {
            destination = PlayerList[0].Data.position;
        }
        break;
        case HuntingState::Attacking:
        {

        }
        break;
    }
}

} // server
