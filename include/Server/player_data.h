#pragma once
#include <SFML/Network/TcpSocket.hpp>
#include <string>
#include <memory>

struct PlayerData
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
