#pragma once
#include <SFML/Network/TcpListener.hpp>
#include "player_data.h"
#include <vector>

class Server
{
public:
    Server();
    bool Update();

private:
    sf::TcpListener listener;
    std::vector<PlayerData> players;
    int owner = -1;

    void listen();
    void checkMessages(PlayerData& player);

    bool setName(PlayerData& player);
    void notifyAllPlayerJoined(PlayerData& p);
    void notifyAllPlayerLeft(PlayerData& p);
    void notifyAllOwnerLeft();
    void listPlayersInLobby(PlayerData& p);
};
