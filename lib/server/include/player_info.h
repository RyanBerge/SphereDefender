/**************************************************************************************************
 *  File:       player_info.h
 *  Class:      PlayerInfo
 *
 *  Purpose:    Player information as needed by the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <string>
#include <memory>

namespace server {

struct PlayerInfo
{
    enum class Status
    {
        Uninitialized,
        Disconnected,
        Menus,
        Alive
    };

    std::string name;
    std::shared_ptr<sf::TcpSocket> socket;
    uint16_t id;
    Status status;
};

} // server
