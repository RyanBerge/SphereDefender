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
#include "entity_data.h"
#include "player.h"
#include "region.h"
#include <memory>
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

    enum class VotingType : uint8_t
    {
        RegionSelect,
        MenuEvent
    };

    Server();
    void Start();

private:
    bool running = false;

    sf::TcpListener listener;
    uint16_t owner = 0;
    GameState game_state = GameState::Uninitialized;
    sf::Clock clock;
    definitions::Zone zone;
    Region region;
    uint16_t current_region;
    uint16_t next_region;
    std::array<definitions::ItemType, 24> item_stash;
    definitions::MenuEvent current_event;

    void update();
    void listen();
    void checkMessages(Player& player);

    void startGame();
    void gatherPlayers();
    void resetVotes();
    void checkVotes(VotingType voting_type);
    void advanceMenuEvent(uint16_t winner);

    uint16_t getPlayerUid();

    void initLobby(Player& player);
    void playerJoined(Player& player);
    void changePlayerProperty(Player& player);
    void startLoading(Player& player);
    void loadingComplete(Player& player);
    void leaveGame(Player& player);
    void updatePlayerState(Player& player);
    void startPlayerAction(Player& player);
    void useItem(Player& player);
    void swapItem(Player& player);
    void castVote(Player& player);
    void consoleInteract(Player& player);

    void broadcastStates();
};

} // server
