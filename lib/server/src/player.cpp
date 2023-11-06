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
#include "definitions.h"
#include "util.h"
#include "messaging.h"
#include "global_state.h"
#include <cmath>
#include <iostream>
#include "SFML/Graphics/Vertex.hpp"

using std::cout, std::endl;
using network::ClientMessage, network::ServerMessage;
using server::global::PlayerList;

namespace server {
namespace {
    constexpr int MEDPACK_HEAL_VALUE = 60;
    constexpr float KNOCKBACK_UNITS_PER_SECOND = 350;

    bool checkForCollisions(sf::FloatRect target, std::vector<sf::FloatRect>& obstacles, sf::FloatRect bounds)
    {
        for (auto& obstacle : obstacles)
        {
            if (util::Intersects(target, obstacle))
            {
                return true;
            }
        }

        if (!util::Intersects(target, bounds))
        {
            return true;
        }

        return false;
    }
}

Player::Player() : definition{definitions::PlayerDefinition::Get()} { }

void Player::Update(sf::Time elapsed, Region& region)
{
    if (global::Paused)
    {
        return;
    }

    projectile_timer += elapsed.asSeconds();
    attack_timer += elapsed.asSeconds();
    movement_override_timer += elapsed.asSeconds();

    processIncomingAttacks();
    handleMovement(elapsed, region);

    if (Attacking)
    {
        handleAttack(elapsed, region);
    }
}

void Player::UpdatePlayerState(sf::Vector2i movement_vector)
{
    if (movement_override)
    {
        return;
    }

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

bool Player::StartAttack(uint16_t attack_angle)
{
    if (movement_override)
    {
        return false;
    }

    starting_attack_angle = attack_angle;
    current_attack_angle = (starting_attack_angle - (definitions::GetWeapon(definitions::WeaponType::Sword).arc / 2)) - 5;
    projectile_timer = 0;
    attack_timer = 0;
    Attacking = true;

    return true;
}

sf::FloatRect Player::GetBounds()
{
    return getBoundingBox(Data.position);
}

util::LineSegment Player::GetSwordLocation()
{
    util::LineSegment sword;

    sword.p1.x = weapon.offset * std::cos(util::ToRadians(starting_attack_angle)) + Data.position.x;
    sword.p1.y = weapon.offset * std::sin(util::ToRadians(starting_attack_angle)) + Data.position.y;
    sword.p2.x = weapon.length * std::cos(util::ToRadians(current_attack_angle)) + sword.p1.x;
    sword.p2.y = weapon.length * std::sin(util::ToRadians(current_attack_angle)) + sword.p1.y;
    return sword;
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

bool Player::SpawnProjectile(definitions::Projectile& out_projectile)
{
    static uint16_t projectile_id = 0;

    if (spawn_projectile)
    {
        sf::Vector2f attack_vector = util::AngleToVector(current_attack_angle + util::GetRandomInt(-weapon.projectile_spread / 2, weapon.projectile_spread / 2));
        definitions::Projectile projectile;
        projectile.id = projectile_id++;
        projectile.velocity = attack_vector * static_cast<float>(weapon.projectile_speed);
        projectile.position = Data.position - attack_vector * static_cast<float>(weapon.offset);
        projectile.owner = Data.id;
        projectile.hostile = false;
        projectile.damage = weapon.damage;
        projectile.knockback = weapon.knockback;
        projectile.invulnerability_window = weapon.invulnerability_window;

        out_projectile = projectile;
        ++projectiles_fired;
        spawn_projectile = false;
        if (projectiles_fired == weapon.projectiles_per_attack)
        {
            Attacking = false;
            projectiles_fired = 0;
        }

        return true;
    }

    return false;
}

definitions::ItemType Player::UseItem()
{
    switch (equipped_item)
    {
        case definitions::ItemType::None:
        {
            return equipped_item;
        }
        break;
        case definitions::ItemType::Medpack:
        {
            Data.health += MEDPACK_HEAL_VALUE;
            if (Data.health > 100)
            {
                Data.health = 100;
            }
        }
        break;
    }

    definitions::ItemType item = equipped_item;
    equipped_item = definitions::ItemType::None;
    ServerMessage::ChangeItem(*Socket, equipped_item);
    return item;
}

definitions::ItemType Player::ChangeItem(definitions::ItemType item)
{
    definitions::ItemType temp = equipped_item;
    equipped_item = item;
    return temp;
}

void Player::AddIncomingAttack(definitions::AttackEvent attack)
{
    attack_events.push(attack);
}

void Player::handleMovement(sf::Time elapsed, Region& region)
{
    sf::Vector2f step = velocity * elapsed.asSeconds();

    if (movement_override)
    {
        if (movement_override_timer >= movement_override_time)
        {
            movement_override = false;
        }
        else
        {
            step = movement_override_vector * elapsed.asSeconds();
        }
    }

    takeStep(step, region);
}

void Player::handleAttack(sf::Time elapsed, Region& region)
{
    switch (Data.properties.weapon_type)
    {
        case definitions::WeaponType::Sword:
        {
            current_attack_angle += weapon.arc_speed * elapsed.asSeconds();

            if (attack_timer >= weapon.animation_time)
            {
                Attacking = false;
                attack_timer = 0;
                return;
            }
            else if (attack_timer >= static_cast<float>(weapon.arc) / weapon.arc_speed)
            {
                // Only deal damage during the swinging part
                return;
            }

            util::LineSegment sword = GetSwordLocation();

            for (auto& enemy : region.Enemies)
            {
                sf::FloatRect bounds = enemy.GetBounds();

                if (util::Intersects(bounds, sword))
                {
                    enemy.WeaponHit(Data.id, weapon.damage, weapon.knockback, enemy.GetData().position - Data.position, weapon.invulnerability_window);
                }
            }
        }
        break;
        case definitions::WeaponType::BurstGun:
        {
            if (projectiles_fired < weapon.projectiles_per_attack && projectile_timer * 1000 >= weapon.delay_per_projectile)
            {
                spawn_projectile = true;
                projectile_timer = 0;
            }
        }
        break;
        case definitions::WeaponType::HitscanGun:
        {
            sf::Vector2f attack_vector = util::AngleToVector(current_attack_angle);

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

            uint16_t target_enemy_id = region.Enemies.front().GetData().id;
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
                        target_enemy_id = enemy.GetData().id;
                    }
                    else if (util::Distance(Data.position, temp) < util::Distance(Data.position, point))
                    {
                        point = temp;
                        enemy_hit = true;
                        target_enemy_id = enemy.GetData().id;
                    }
                }
            }

            if (enemy_hit)
            {
                Enemy& enemy = GetEnemyById(target_enemy_id, region.Enemies);
                enemy.WeaponHit(Data.id, weapon.damage, weapon.knockback, enemy.GetData().position - Data.position, weapon.invulnerability_window);
            }

            Attacking = false;
        }
        break;
    }
}

void Player::takeStep(sf::Vector2f step, Region& region)
{
    bool collision = false;
    sf::Vector2f new_position = Data.position;
    if (!Attacking)
    {
        new_position = Data.position + step;
        collision = checkForCollisions(getBoundingBox(new_position), region.Obstacles, region.Bounds);
    }

    if (collision)
    {
        collision = false;
        sf::Vector2f partial{new_position.x, Data.position.y};
        collision = checkForCollisions(getBoundingBox(partial), region.Obstacles, region.Bounds);

        if (!collision)
        {
            new_position = partial;
        }
    }

    if (collision)
    {
        collision = false;
        sf::Vector2f partial{Data.position.x, new_position.y};
        collision = checkForCollisions(getBoundingBox(partial), region.Obstacles, region.Bounds);

        if (!collision)
        {
            new_position = partial;
        }
    }

    if (!collision)
    {
        Data.position = new_position;
    }
}

void Player::step(sf::Time elapsed, Region& region)
{
    bool collision = false;
    sf::Vector2f new_position = Data.position;
    if (!Attacking)
    {
        new_position = Data.position + velocity * elapsed.asSeconds();
        collision = checkForCollisions(getBoundingBox(new_position), region.Obstacles, region.Bounds);
    }

    if (collision)
    {
        collision = false;
        sf::Vector2f partial{new_position.x, Data.position.y};
        collision = checkForCollisions(getBoundingBox(partial), region.Obstacles, region.Bounds);

        if (!collision)
        {
            new_position = partial;
        }
    }

    if (collision)
    {
        collision = false;
        sf::Vector2f partial{Data.position.x, new_position.y};
        collision = checkForCollisions(getBoundingBox(partial), region.Obstacles, region.Bounds);

        if (!collision)
        {
            new_position = partial;
        }
    }

    if (!collision)
    {
        Data.position = new_position;
    }
}

void Player::processIncomingAttacks()
{
    while (!attack_events.empty())
    {
        definitions::AttackEvent event = attack_events.front();
        attack_events.pop();
        Damage(event.attack.damage);
        movement_override_time = event.attack.knockback_distance / KNOCKBACK_UNITS_PER_SECOND;
        movement_override_timer = 0;
        if (event.attack.knockback_distance != 0)
        {
            movement_override = true;
            movement_override_vector = util::Normalize(Data.position - event.origin) * KNOCKBACK_UNITS_PER_SECOND;
            network::PlayerAction action;
            action.type = network::PlayerActionType::Stunned;
            action.duration = movement_override_time;
            startPlayerAction(action);
        }
    }
}

void Player::startPlayerAction(network::PlayerAction action)
{
    for (auto& p : PlayerList)
    {
        ServerMessage::PlayerStartAction(*p.Socket, Data.id, action);
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
