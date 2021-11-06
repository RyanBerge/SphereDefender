/**************************************************************************************************
 *  File:       game_manager.cpp
 *  Class:      GameManager
 *
 *  Purpose:    GameManager is the top-level state manager for the client
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <SFML/Window/Event.hpp>
#include <functional>
#include "game_manager.h"
#include "event_handler.h"

namespace {
    // TODO: Move to Settings
    const sf::Vector2f DEFAULT_RATIO = { 1200, 800 };
}

namespace client {

GameManager::GameManager() { }

GameManager& GameManager::GetInstance()
{
    static GameManager manager;
    return manager;
}

void GameManager::Start()
{
    EventHandler& event_handler = EventHandler::GetInstance();
    event_handler.RegisterCallback(sf::Event::EventType::Resized, std::bind(&GameManager::onResizeWindow, this, std::placeholders::_1));
    event_handler.RegisterCallback(sf::Event::EventType::Closed, std::bind(&GameManager::onCloseWindow, this, std::placeholders::_1));

    Window.create(sf::VideoMode(DEFAULT_RATIO.x, DEFAULT_RATIO.y), "Sphere Defender");
    sf::Clock clock;

    running = true;

    while (running && Window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        event_handler.RunCallbacks();

        // TODO: Will we need to update one of these even outside of its state?
        switch (State)
        {
            case GameState::MainMenu:
            {
                //MainMenu.Update(elapsed);
            }
            break;
            case GameState::Game:
            {
                (void)elapsed;
                // TODO: Game
            }
            break;
        }

        Window.clear(sf::Color::Black);

        // TODO: Will we need to update one of these even outside of its state?
        switch (State)
        {
            case GameState::MainMenu:
            {
                MainMenu.Draw();
            }
            break;
            case GameState::Game:
            {
                // TODO: Game
            }
            break;
        }

        Window.display();

        sf::Event event;
        while (Window.pollEvent(event))
        {
            event_handler.AddEvent(event);
        }
    }
}

void GameManager::ExitGame()
{
    Window.close();
    running = false;
}

void GameManager::onCloseWindow(sf::Event event)
{
    (void)event;
    ExitGame();
}

void GameManager::onResizeWindow(sf::Event event)
{
    sf::Vector2f ratio = sf::Vector2f(event.size.width, event.size.height);

    float aspect_ratio = DEFAULT_RATIO.x / DEFAULT_RATIO.y;
    float window_ratio = (ratio.x / ratio.y);

    float viewport_width = 1;
    float viewport_height = 1;
    float viewport_x = 0;
    float viewport_y = 0;

    if (window_ratio > aspect_ratio)
    {
        viewport_width = aspect_ratio / window_ratio;
        viewport_x = (1 - viewport_width) / 2;
    }
    else if (window_ratio < aspect_ratio)
    {
        viewport_height = window_ratio / aspect_ratio;
        viewport_y = (1 - viewport_height) / 2;
    }

    sf::FloatRect view_rect{0, 0, DEFAULT_RATIO.x, DEFAULT_RATIO.y};
    view_rect.left = Window.getView().getCenter().x - Window.getView().getSize().x / 2;
    view_rect.top = Window.getView().getCenter().y - Window.getView().getSize().y / 2;

    sf::View view(view_rect);
    view.setViewport(sf::FloatRect(viewport_x, viewport_y, viewport_width, viewport_height));
    Window.setView(view);
}

} // client
