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
#include "messaging.h"

namespace server {

class PlayerInfo
{
public:
    enum class PlayerStatus
    {
        Uninitialized,
        Disconnected,
        Menus,
        Alive
    };

    void Update(sf::Time elapsed);
    void UpdatePlayerState(sf::Vector2i movement_vector);
    void StartAttack(uint16_t attack_angle);

    std::shared_ptr<sf::TcpSocket> Socket;
    PlayerStatus Status;
    network::PlayerData Data;

private:
    double movement_speed = 200; // pixels per second
    int swing_speed = 360; // degrees per second
    int swing_arc = 90; // degrees

    sf::Vector2f velocity;
    bool attacking = false;
    double starting_attack_angle;
    double current_attack_angle;

};

} // server
