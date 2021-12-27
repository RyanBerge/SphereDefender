/**************************************************************************************************
 *  File:       player_info.h
 *  Class:      Player
 *
 *  Purpose:    The representation of players from the server's point-of-view
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/

#include "player.h"
#include "game_math.h"
#include "util.h"
#include <cmath>
#include <iostream>
#include "SFML/Graphics/Vertex.hpp"

using std::cout, std::endl;

namespace server {

Player::Player() : definition{definitions::PlayerDefinition::Get()} { }

void Player::Update(sf::Time elapsed, Region& region)
{
    sf::Vector2f new_position = Data.position + velocity * elapsed.asSeconds();
    bool collision = false;
    for (auto& obstacle : region.Obstacles)
    {
        if (util::Intersects(getBoundingBox(new_position), obstacle))
        {
            collision = true;
            break;
        }
    }

    if (collision)
    {
        collision = false;
        sf::Vector2f partial{new_position.x, Data.position.y};
        for (auto& obstacle : region.Obstacles)
        {
            if (util::Intersects(getBoundingBox(partial), obstacle))
            {
                collision = true;
                break;
            }
        }

        if (!collision)
        {
            new_position = partial;
        }
    }

    if (collision)
    {
        collision = false;
        sf::Vector2f partial{Data.position.x, new_position.y};
        for (auto& obstacle : region.Obstacles)
        {
            if (util::Intersects(getBoundingBox(partial), obstacle))
            {
                collision = true;
                break;
            }
        }

        if (!collision)
        {
            new_position = partial;
        }
    }

    if (!collision)
    {
        Data.position = new_position;
    }

    if (Attacking)
    {
        handleAttack(elapsed, region);
    }
}

void Player::UpdatePlayerState(sf::Vector2i movement_vector)
{
    double hyp = std::hypot(movement_vector.x, movement_vector.y);

    if (hyp == 0)
    {
        velocity.x = 0;
        velocity.y = 0;
    }
    else
    {
        velocity.x = (movement_vector.x / hyp) * definition.speed;
        velocity.y = (movement_vector.y / hyp) * definition.speed;
    }
}

void Player::StartAttack(uint16_t attack_angle)
{
    starting_attack_angle = attack_angle;
    current_attack_angle = starting_attack_angle;
    Attacking = true;
}

sf::FloatRect Player::GetBounds()
{
    return getBoundingBox(Data.position);
}

util::LineSegment Player::GetSwordLocation()
{
    util::LineSegment sword;
    sword.p1.x = -weapon.offset * std::cos(current_attack_angle * util::pi / 180) + Data.position.x;
    sword.p1.y = -weapon.offset * std::sin(current_attack_angle * util::pi / 180) + Data.position.y;
    sword.p2.x = (-weapon.offset + weapon.length) * std::cos(current_attack_angle * util::pi / 180) + Data.position.x;
    sword.p2.y = (-weapon.offset + weapon.length) * std::sin(current_attack_angle * util::pi / 180) + Data.position.y;
    return sword;
}

sf::Vector2f Player::GetAttackVector()
{
    return sf::Vector2f{static_cast<float>(std::cos(current_attack_angle * util::pi / 180)), static_cast<float>(std::sin(current_attack_angle * util::pi / 180))};
}

definitions::Weapon Player::GetWeapon()
{
    return weapon;
}

void Player::SetWeapon(definitions::Weapon new_weapon)
{
    weapon = new_weapon;
}

void Player::Damage(int damage_value)
{
    if (Data.health < damage_value)
    {
        Data.health = 0;
        Status = PlayerStatus::Dead;
    }
    else
    {
        Data.health -= damage_value;
    }
}

void Player::handleAttack(sf::Time elapsed, Region& region)
{
    switch (Data.properties.weapon_type)
    {
        case definitions::WeaponType::Sword:
        {
            current_attack_angle = std::fmod(current_attack_angle + weapon.arc_speed * elapsed.asSeconds(), 360);
            float rotation_delta = std::fmod(current_attack_angle - starting_attack_angle + 360, 360);

            if (rotation_delta > weapon.arc)
            {
                Attacking = false;
            }

            util::LineSegment sword = GetSwordLocation();

            for (auto& enemy : region.Enemies)
            {
                sf::FloatRect bounds = enemy.GetBounds();

                if (util::Contains(bounds, sword.p1) || util::Contains(bounds, sword.p2))
                {
                    enemy.WeaponHit(Data.id, weapon.damage, weapon.knockback, enemy.Data.position - Data.position);
                }
            }
        }
        break;
        case definitions::WeaponType::BurstGun:
        case definitions::WeaponType::HitscanGun:
        {
            sf::Vector2f attack_vector = GetAttackVector();

            bool collision = false;
            sf::Vector2f point;
            for (auto& rect : region.Obstacles)
            {
                sf::Vector2f temp;
                if (util::IntersectionPoint(rect, util::LineVector{Data.position, attack_vector}, temp))
                {
                    if (!collision)
                    {
                        collision = true;
                        point = temp;
                    }
                    else if (util::Distance(Data.position, temp) < util::Distance(Data.position, point))
                    {
                        point = temp;
                    }
                }
            }

            uint16_t target_enemy_id = region.Enemies.front().Data.id;
            bool enemy_hit = false;
            for (auto& enemy : region.Enemies)
            {
                sf::Vector2f temp;
                if (util::IntersectionPoint(enemy.GetBounds(), util::LineVector{Data.position, attack_vector}, temp))
                {
                    if (!collision)
                    {
                        collision = true;
                        point = temp;
                        enemy_hit = true;
                        target_enemy_id = enemy.Data.id;
                    }
                    else if (util::Distance(Data.position, temp) < util::Distance(Data.position, point))
                    {
                        point = temp;
                        enemy_hit = true;
                        target_enemy_id = enemy.Data.id;
                    }
                }
            }

            if (enemy_hit)
            {
                Enemy& enemy = GetEnemyById(target_enemy_id, region.Enemies);
                if (enemy.Data.health <= 10)
                {
                    enemy.Data.health = 0;
                }
                else
                {
                    enemy.WeaponHit(Data.id, weapon.damage, weapon.knockback, enemy.Data.position - Data.position);
                }
            }

            Attacking = false;
        }
        break;
    }
}

sf::FloatRect Player::getBoundingBox(sf::Vector2f position)
{
    sf::FloatRect rect;
    rect.width = definition.radius * 2;
    rect.height = definition.radius * 2;
    rect.left = position.x - definition.radius;
    rect.top = position.y - definition.radius;

    return rect;
}

} // server
