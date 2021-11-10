/**************************************************************************************************
 *  File:       game.h
 *  Class:      Game
 *
 *  Purpose:    The top-level controller for everything in-game
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "game.h"
#include "messaging.h"
#include "game_manager.h"
#include <thread>
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Game::Game() { }

void Game::Update(sf::Time elapsed)
{
    (void)elapsed;
}

void Game::Draw()
{
    GameManager::GetInstance().Window.draw(sphere);
}

void Game::Load()
{
    loaded = false;
    std::thread loading_thread(asyncLoad, this);
    loading_thread.detach();
}

void Game::asyncLoad()
{
    std::cout << "Async load started..." << std::endl;

    sphere.setFillColor(sf::Color(100, 250, 250));
    sphere.setRadius(100);
    sphere.setPosition(300, 300);

    loaded = true;
    std::cout << "Async load finished." << std::endl;

    ClientMessage::LoadingComplete(ServerSocket);
}

} // client
