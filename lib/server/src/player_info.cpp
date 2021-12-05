/**************************************************************************************************
 *  File:       player_info.h
 *  Class:      PlayerInfo
 *
 *  Purpose:    The representation of players from the server's point-of-view
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/

#include "player_info.h"
#include "game_math.h"
#include <cmath>
#include <iostream>
#include "SFML/Graphics/Vertex.hpp"

using std::cout, std::endl;

namespace server {

namespace {
    const int PLAYER_WIDTH = 70;
    const int PLAYER_HEIGHT = 70;
}

void PlayerInfo::Update(sf::Time elapsed, std::vector<sf::FloatRect> obstacles)
{
    sf::Vector2f new_position = Data.position + velocity * elapsed.asSeconds();
    bool collision = false;
    for (auto& obstacle : obstacles)
    {
        if (util::Intersects(GetBoundingBox(new_position), obstacle))
        {
            // Try it with only a partial movement vector
            sf::Vector2f partial_x{new_position.x, Data.position.y};
            sf::Vector2f partial_y{Data.position.x, new_position.y};
            if (!util::Intersects(GetBoundingBox(partial_x), obstacle))
            {
                new_position = partial_x;
            }
            else if (!util::Intersects(GetBoundingBox(partial_y), obstacle))
            {
                new_position = partial_y;
            }
            else
            {
                collision = true;
                break;
            }
        }
    }

    if (!collision)
    {
        Data.position = new_position;
    }

    if (Attacking)
    {
        switch (Data.properties.player_class)
        {
            case network::PlayerClass::Melee:
            {
                current_attack_angle = std::fmod(current_attack_angle + swing_speed * elapsed.asSeconds(), 360);
                float rotation_delta = std::fmod(current_attack_angle - starting_attack_angle + 360, 360);

                if (rotation_delta > swing_arc)
                {
                    Attacking = false;
                }
            }
            break;
            default:
            {
            }
        }
    }
}

void PlayerInfo::UpdatePlayerState(sf::Vector2i movement_vector)
{
    double hyp = std::hypot(movement_vector.x, movement_vector.y);

    if (hyp == 0)
    {
        velocity.x = 0;
        velocity.y = 0;
    }
    else
    {
        velocity.x = (movement_vector.x / hyp) * movement_speed;
        velocity.y = (movement_vector.y / hyp) * movement_speed;
    }
}

void PlayerInfo::StartAttack(uint16_t attack_angle)
{
    starting_attack_angle = attack_angle;
    current_attack_angle = starting_attack_angle;
    Attacking = true;
}

util::LineSegment PlayerInfo::GetSwordLocation()
{
    util::LineSegment sword;
    sword.p1.x = sword_offset * std::cos(current_attack_angle * util::pi / 180) + Data.position.x;
    sword.p1.y = sword_offset * std::sin(current_attack_angle * util::pi / 180) + Data.position.y;
    sword.p2.x = (sword_offset + sword_length) * std::cos(current_attack_angle * util::pi / 180) + Data.position.x;
    sword.p2.y = (sword_offset + sword_length) * std::sin(current_attack_angle * util::pi / 180) + Data.position.y;
    return sword;
}

sf::Vector2f PlayerInfo::GetAttackVector()
{
    return sf::Vector2f{static_cast<float>(std::cos(current_attack_angle * util::pi / 180)), static_cast<float>(std::sin(current_attack_angle * util::pi / 180))};
}

uint8_t PlayerInfo::GetWeaponDamage()
{
    switch (Data.properties.player_class)
    {
        case network::PlayerClass::Melee:
        {
            return 50;
        }
        break;
        case network::PlayerClass::Ranged:
        {
            return 20;
        }
        break;
        default:
        {
            return 0;
        }
    }
}

PlayerInfo::WeaponKnockback PlayerInfo::GetWeaponKnockback()
{
    switch (Data.properties.player_class)
    {
        case network::PlayerClass::Melee:
        {
            return WeaponKnockback{50, 100};
        }
        break;
        default:
        {
            return WeaponKnockback{0, 0};
        }
        break;
    }
}

void PlayerInfo::Damage(int damage_value)
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

sf::FloatRect PlayerInfo::GetBoundingBox(sf::Vector2f position)
{
    sf::FloatRect rect;
    rect.width = PLAYER_WIDTH;
    rect.height = PLAYER_HEIGHT;
    rect.left = position.x - PLAYER_WIDTH / 2;
    rect.top = position.y - PLAYER_HEIGHT / 2;

    return rect;
}

} // server
