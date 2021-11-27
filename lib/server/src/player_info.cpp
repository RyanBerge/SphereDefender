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
#include <cmath>

namespace server {

void PlayerInfo::Update(sf::Time elapsed)
{
    Data.position += velocity * elapsed.asSeconds();

    current_attack_angle = std::fmod(swing_speed * elapsed.asSeconds(), 360);
    float rotation_delta = std::fmod(current_attack_angle - starting_attack_angle + 360, 360);

    if (rotation_delta > swing_arc)
    {
        attacking = false;
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
    attacking = true;
}

} // server
