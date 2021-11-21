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
#include <SFML/System/Clock.hpp>
#include "messaging.h"
#include <memory>
#include <vector>

namespace server {

class Server
{
public:
    struct PlayerInfo
    {
        enum class Status
        {
            Uninitialized,
            Disconnected,
            Menus,
            Alive
        };

        std::shared_ptr<sf::TcpSocket> socket;
        Status status;
        network::PlayerData data;
        sf::Vector2f velocity;
        double movement_speed = 200; // pixels per second
    };

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
    sf::Clock clock;

    void update();

    void listen();
    void checkMessages(PlayerInfo& player);

    uint16_t getPlayerUid();

    void initLobby(PlayerInfo& player);
    void playerJoined(PlayerInfo& player);
    void startGame(PlayerInfo& player);
    void loadingComplete(PlayerInfo& player);
    void leaveGame(PlayerInfo& player);
    void updatePlayerState(PlayerInfo& player);

    void broadcastStates();
};

} // server
