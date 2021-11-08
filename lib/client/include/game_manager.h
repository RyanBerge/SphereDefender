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

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include "main_menu.h"

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

    // TODO: Should this just be a map of menus instead of a state enum?
    GameState State = GameState::MainMenu;

    sf::RenderWindow Window;
    MainMenu MainMenu;
    sf::TcpSocket ServerSocket;

    void Start();
    void ExitGame();

private:
    GameManager();

    void onCloseWindow(sf::Event event);
    void onResizeWindow(sf::Event event);

    bool running = false;
};

} // client
