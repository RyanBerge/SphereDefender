#pragma once
#include <SFML/Network/TcpListener.hpp>
#include "player_data.h"
#include <vector>

class Server
{
public:
    Server();
    void Update();

private:
    sf::TcpListener listener;
    std::vector<PlayerData> players;

    void listen();
    void checkMessages(PlayerData& player);

    void setName(PlayerData& player);
};
