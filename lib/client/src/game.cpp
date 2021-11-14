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
#include "event_handler.h"
#include <thread>
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

// TODO: Move view sizze to Settings
Game::Game() : world_view(sf::FloatRect(0, 0, 1200, 800)), scroll_data{0, 0} { }

void Game::Update(sf::Time elapsed)
{
    (void)elapsed;
    if (loaded)
    {
        // TODO: Move to Settings
        float scroll_speed = 400;
        float horizontal_scroll = scroll_data.horizontal * scroll_speed * elapsed.asSeconds();
        float vertical_scroll = scroll_data.vertical * scroll_speed * elapsed.asSeconds();
        world_view.move(horizontal_scroll, vertical_scroll);
    }
}

void Game::Draw()
{
    if (loaded)
    {
        sf::View old_view = GameManager::GetInstance().Window.getView();
        world_view.setViewport(old_view.getViewport());
        GameManager::GetInstance().Window.setView(world_view);

        world_map.Draw();

        GameManager::GetInstance().Window.setView(old_view);

        gui.Draw();
    }
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

    mouse_move_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&Game::onMouseMove, this, std::placeholders::_1));
    mouse_down_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Game::onMouseDown, this, std::placeholders::_1));
    mouse_up_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Game::onMouseUp, this, std::placeholders::_1));
    text_entered_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&Game::onTextEntered, this, std::placeholders::_1));
    key_pressed_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyPressed, std::bind(&Game::onKeyPressed, this, std::placeholders::_1));
    key_released_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyReleased, std::bind(&Game::onKeyReleased, this, std::placeholders::_1));

    world_map.Load();
    gui.Load();

    world_view = sf::View(sf::FloatRect(0, 0, 1200, 800));
    loaded = true;
    std::cout << "Async load finished." << std::endl;

    ClientMessage::LoadingComplete(ServerSocket);
}

void Game::Unload()
{
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseMoved, mouse_move_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseButtonPressed, mouse_down_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseButtonReleased, mouse_up_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::TextEntered, text_entered_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::KeyPressed, key_pressed_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::KeyReleased, key_released_id);

    world_map.Unload();
    gui.Unload();
}

void Game::onMouseMove(sf::Event event)
{
    gui.OnMouseMove(event);
}

void Game::onMouseDown(sf::Event event)
{
    gui.OnMouseDown(event);
}

void Game::onMouseUp(sf::Event event)
{
    gui.OnMouseUp(event);
}

void Game::onTextEntered(sf::Event event)
{
    gui.OnTextEntered(event);
}

void Game::onKeyPressed(sf::Event event)
{
    // TODO: Move key to Settings
    switch (event.key.code)
    {
        case sf::Keyboard::Key::W:
        {
            --scroll_data.vertical;
        }
        break;
        case sf::Keyboard::Key::A:
        {
            --scroll_data.horizontal;
        }
        break;
        case sf::Keyboard::Key::S:
        {
            ++scroll_data.vertical;
        }
        break;
        case sf::Keyboard::Key::D:
        {
            ++scroll_data.horizontal;
        }
        break;
        default: { }
    }
}

void Game::onKeyReleased(sf::Event event)
{
    // TODO: Move key to Settings
    switch (event.key.code)
    {
        case sf::Keyboard::Key::W:
        {
            ++scroll_data.vertical;
        }
        break;
        case sf::Keyboard::Key::A:
        {
            ++scroll_data.horizontal;
        }
        break;
        case sf::Keyboard::Key::S:
        {
            --scroll_data.vertical;
        }
        break;
        case sf::Keyboard::Key::D:
        {
            --scroll_data.horizontal;
        }
        break;
        default: { }
    }
}



} // client
