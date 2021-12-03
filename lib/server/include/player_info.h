/**************************************************************************************************
 *  File:       player_info.h
 *  Class:      PlayerInfo
 *
 *  Purpose:    The representation of players from the server's point-of-view
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <memory>
#include "entity_data.h"
#include "game_math.h"

namespace server {

class PlayerInfo
{
public:
    enum class PlayerStatus
    {
        Uninitialized,
        Disconnected,
        Menus,
        Alive,
        Dead
    };

    void Update(sf::Time elapsed, std::vector<sf::FloatRect> obstacles);
    void UpdatePlayerState(sf::Vector2i movement_vector);
    void StartAttack(uint16_t attack_angle);
    util::LineSegment GetSwordLocation();
    void Damage(int damage_value);

    std::shared_ptr<sf::TcpSocket> Socket;
    PlayerStatus Status;
    network::PlayerData Data;

    bool Attacking = false;

private:
    sf::FloatRect GetBoundingBox(sf::Vector2f position);

    double movement_speed = 200; // pixels per second
    int swing_speed = 360; // degrees per second
    int swing_arc = 90; // degrees
    int sword_offset = 35;
    int sword_length = 40;

    sf::Vector2f velocity;
    double starting_attack_angle;
    double current_attack_angle;

};

} // server
