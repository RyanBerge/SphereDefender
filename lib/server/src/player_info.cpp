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

void PlayerInfo::Update(sf::Time elapsed)
{
    Data.position += velocity * elapsed.asSeconds();

    if (Attacking)
    {
        current_attack_angle = std::fmod(current_attack_angle + swing_speed * elapsed.asSeconds(), 360);
        float rotation_delta = std::fmod(current_attack_angle - starting_attack_angle + 360, 360);

        //cout << "Angle: " << current_attack_angle << ", Delta: " << rotation_delta << endl;
        //cout << current_attack_angle << endl;

        if (rotation_delta > swing_arc)
        {
            Attacking = false;
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

} // server
