/**************************************************************************************************
 *  File:       enemy.cpp
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "enemy.h"
#include <iostream>

using std::cout, std::endl;

namespace server {

namespace {
    const int AGGRO_THRESHOLD = 350; // pixels
}

Enemy::Enemy()
{
    // TODO: Enemies should center their position the way players do
    static uint16_t identifier = 0;
    Data.id = identifier++;
    Data.health = 100;
    Data.type = network::EnemyData::EnemyType::SmallDemon;
}

sf::FloatRect Enemy::GetBounds()
{
    return sf::FloatRect(Data.position.x - 25, Data.position.y - 25, 50, 50);
}

void Enemy::Update(sf::Time elapsed, std::vector<PlayerInfo>& players, network::ConvoyData convoy)
{
    if (!attacking)
    {
        move(elapsed, players, convoy);
    }
    else
    {
        sf::Vector2f velocity;
        double hyp = std::hypot(attack_vector.x, attack_vector.y);

        if (attack_timer.getElapsedTime().asMilliseconds() < attack_duration / 2)
        {
            velocity.x = (attack_vector.x / hyp) * movement_speed * 1.25;
            velocity.y = (attack_vector.y / hyp) * movement_speed * 1.25;
            Data.position += velocity * elapsed.asSeconds();
        }
        else if (attack_timer.getElapsedTime().asMilliseconds() < attack_duration)
        {
            if (!attack_flag)
            {
                checkAttack(players, convoy);
                attack_flag = true;
            }
            velocity.x = (-attack_vector.x / hyp) * movement_speed * 1.25;
            velocity.y = (-attack_vector.y / hyp) * movement_speed * 1.25;
            Data.position += velocity * elapsed.asSeconds();
        }
        if (attack_timer.getElapsedTime().asMilliseconds() >= attack_duration + attack_cooldown)
        {
            attacking = false;
            Data.position = starting_attack_position;
        }
    }
}

void Enemy::move(sf::Time elapsed, std::vector<PlayerInfo>& players, network::ConvoyData convoy)
{
    sf::Vector2f target;
    sf::FloatRect bounds;
    bool player_target = false;

    if (convoy.orientation == network::ConvoyData::Orientation::North || convoy.orientation == network::ConvoyData::Orientation::South)
    {
        bounds = sf::FloatRect(convoy.position.x - convoy.WIDTH / 2, convoy.position.y - convoy.HEIGHT / 2, convoy.WIDTH, convoy.HEIGHT);
    }
    else if (convoy.orientation == network::ConvoyData::Orientation::East || convoy.orientation == network::ConvoyData::Orientation::West)
    {
        bounds = sf::FloatRect(convoy.position.x - convoy.HEIGHT / 2, convoy.position.y - convoy.WIDTH / 2, convoy.HEIGHT, convoy.WIDTH);
    }

    if (Data.position.x >= bounds.left && Data.position.x <= bounds.left + bounds.width)
    {
        if (Data.position.y <= bounds.top)
        {
            target.x = Data.position.x;
            target.y = bounds.top;
        }
        else
        {
            target.x = Data.position.x;
            target.y = bounds.top + bounds.height;
        }
    }
    else if (Data.position.y >= bounds.top && Data.position.y <= bounds.top + bounds.height)
    {
        if (Data.position.x <= bounds.left)
        {
            target.x = bounds.left;
            target.y = Data.position.y;
        }
        else
        {
            target.x = bounds.left + bounds.width;
            target.y = Data.position.y;
        }
    }
    else
    {
        if (Data.position.x <= bounds.left && Data.position.y <= bounds.top) // upper-left
        {
            target.x = bounds.left;
            target.y = bounds.top;
        }
        else if (Data.position.x <= bounds.left && Data.position.y >= bounds.top + bounds.height) // lower-left
        {
            target.x = bounds.left;
            target.y = bounds.top + bounds.height;
        }
        else if (Data.position.x >= bounds.left + bounds.width && Data.position.y <= bounds.top) // upper-right
        {
            target.x = bounds.left + bounds.width;
            target.y = bounds.top;
        }
        else if (Data.position.x >= bounds.left + bounds.width && Data.position.y >= bounds.top + bounds.height) // lower-right
        {
            target.x = bounds.left + bounds.width;
            target.y = bounds.top + bounds.height;
        }
    }

    double origin_distance = util::Distance(Data.position, target);
    double current_distance = origin_distance;

    for (auto& player : players)
    {
        if (player.Status == PlayerInfo::PlayerStatus::Alive)
        {
            double new_distance = util::Distance(Data.position, player.Data.position);
            if (new_distance < current_distance && new_distance < AGGRO_THRESHOLD)
            {
                current_distance = new_distance;
                target = player.Data.position;
                player_target = true;
            }
        }
    }

    sf::Vector2f movement_vector = target - Data.position;
    sf::Vector2f velocity{};

    double hyp = std::hypot(movement_vector.x, movement_vector.y);

    int range_threshold = attack_range;
    if (player_target)
    {
        range_threshold += 35;
    }

    if (hyp < range_threshold)
    {
        velocity.x = 0;
        velocity.y = 0;
        attacking = true;
        attack_flag = false;
        attack_timer.restart();
        attack_vector = sf::Vector2f{static_cast<float>(movement_vector.x / hyp), static_cast<float>(movement_vector.y / hyp)};
        starting_attack_position = Data.position;
    }
    else
    {
        velocity.x = (movement_vector.x / hyp) * movement_speed;
        velocity.y = (movement_vector.y / hyp) * movement_speed;
    }

    Data.position += velocity * elapsed.asSeconds();
}

void Enemy::checkAttack(std::vector<PlayerInfo>& players, network::ConvoyData convoy)
{
    (void)convoy;

    for (auto& player : players)
    {
        if (player.Status == PlayerInfo::PlayerStatus::Alive)
        {
            sf::FloatRect bounds(player.Data.position.x - 35, player.Data.position.y - 35, 70, 70);
            if (bounds.intersects(GetBounds()))
            {
                player.Damage(attack_damage);
            }
        }
    }
}

} // server
