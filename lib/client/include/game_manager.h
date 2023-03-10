/**************************************************************************************************
 *  File:       game_manager.h
 *  Class:      GameManager
 *
 *  Purpose:    GameManager is the top-level state manager for the client
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "main_menu.h"
#include "game.h"

namespace client {

class GameManager
{
public:
    enum class GameState
    {
        MainMenu,
        Game
    };

    GameManager(const GameManager&) = delete;
    GameManager(GameManager&&) = delete;

    GameManager& operator=(const GameManager&) = delete;
    GameManager& operator=(GameManager&&) = delete;

    static GameManager& GetInstance();

    GameState State = GameState::MainMenu;

    client::Game Game;
    client::MainMenu MainMenu;

    void Start();
    void ExitGame();
    void Reset();
    bool ConnectToServer(std::string ip);
    void DisconnectFromServer();

private:
    GameManager();

    void checkMessages();
    void handleDisconnected();

    void onCloseWindow(sf::Event event);
    void onResizeWindow(sf::Event event);

    bool server_connected = false;
    bool running = false;
};

} // client
