#pragma once
#include <SFML/Network/TcpListener.hpp>
#include "player_data.h"
#include <vector>

enum class GameState : uint8_t
{
    Uninitialized,
    Lobby,
    Loading,
    Game
};

class Server
{
public:
    Server();
    bool Update();

private:
    sf::TcpListener listener;
    std::vector<PlayerData> players;
    int owner = -1;
    GameState game_state = GameState::Uninitialized;

    void listen();
    void checkMessages(PlayerData& player);

    bool setName(PlayerData& player);
    void notifyAllPlayerJoined(PlayerData& p);
    void notifyAllPlayerLeft(PlayerData& p);
    void notifyAllOwnerLeft();
    void listPlayersInLobby(PlayerData& p);
    void startGame();
    void notifyAllPlayersLoaded();
};
