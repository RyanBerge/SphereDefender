#include "game.h"
#include "messaging.h"
#include <thread>
#include <iostream>

Game::Game()
{

}

bool Game::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    return true;
}

void Game::Draw(sf::RenderWindow& window)
{
    window.draw(sphere);
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
    Message::LoadingComplete();
}
