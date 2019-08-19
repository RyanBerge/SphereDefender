#pragma once
#include <SFML/Network/TcpSocket.hpp>
#include <string>
#include <memory>

struct PlayerData
{
    std::string name;
    std::shared_ptr<sf::TcpSocket> socket;
    // TODO: PlayerId
};
