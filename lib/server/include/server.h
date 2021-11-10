/**************************************************************************************************
 *  File:       server.h
 *  Class:      Server
 *
 *  Purpose:    The main server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Network/TcpListener.hpp>
#include "player_info.h"
#include <vector>

namespace server {

class Server
{
public:
    enum class GameState : uint8_t
    {
        Uninitialized,
        Lobby,
        Loading,
        Game
    };

    Server();
    void Start();

private:
    bool running = false;

    sf::TcpListener listener;
    std::vector<PlayerInfo> players;
    uint16_t owner = 0;
    GameState game_state = GameState::Uninitialized;

    void update();

    void listen();
    void checkMessages(PlayerInfo& player);

    uint16_t getPlayerUid();

    void initLobby(PlayerInfo& player);
    void playerJoined(PlayerInfo& player);
    void startGame(PlayerInfo& player);
    void loadingComplete(PlayerInfo& player);
    void leaveGame(PlayerInfo& player);
};

} // server
