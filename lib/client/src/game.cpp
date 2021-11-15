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
#include "settings.h"
#include <thread>
#include <iostream>
#include "SFML/System/Sleep.hpp"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Game::Game() : world_view(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y)), scroll_data{0, 0} { }

void Game::Update(sf::Time elapsed)
{
    if (loaded)
    {
        float horizontal_scroll = scroll_data.horizontal * Settings::GetInstance().ScrollSpeed * elapsed.asSeconds();
        float vertical_scroll = scroll_data.vertical * Settings::GetInstance().ScrollSpeed * elapsed.asSeconds();
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

        if (menu_open)
        {
            Settings::GetInstance().Draw();
        }
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

    event_id_map[sf::Event::EventType::MouseMoved] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&Game::onMouseMove, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Game::onMouseDown, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Game::onMouseUp, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::TextEntered] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&Game::onTextEntered, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::KeyPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyPressed, std::bind(&Game::onKeyPressed, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::KeyReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyReleased, std::bind(&Game::onKeyReleased, this, std::placeholders::_1));

    world_map.Load();
    gui.Load();

    world_view = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));

    sf::sleep(sf::seconds(1));

    loaded = true;
    std::cout << "Async load finished." << std::endl;

    ClientMessage::LoadingComplete(ServerSocket);
}

void Game::Unload()
{
    for (auto& event : event_id_map)
    {
        EventHandler::GetInstance().UnregisterCallback(event.first, event.second);
    }

    world_map.Unload();
    gui.Unload();
}

// TODO: Ensure that these are only updated for elements that can be controlled at that time
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
    Settings::KeyBindings bindings = Settings::GetInstance().Bindings;

    if (event.key.code == bindings.ScrollLeft)
    {
        --scroll_data.horizontal;
    }

    if (event.key.code == bindings.ScrollRight)
    {
        ++scroll_data.horizontal;
    }

    if (event.key.code == bindings.ScrollUp)
    {
        --scroll_data.vertical;
    }

    if (event.key.code == bindings.ScrollDown)
    {
        ++scroll_data.vertical;
    }

    if (event.key.code == bindings.Pause)
    {
        if (!menu_open)
        {
            Settings::GetInstance().Open();
            menu_open = true;
        }
        else
        {
            Settings::GetInstance().Close();
            menu_open = false;
        }
    }
}

void Game::onKeyReleased(sf::Event event)
{
    Settings::KeyBindings bindings = Settings::GetInstance().Bindings;

    if (event.key.code == bindings.ScrollLeft)
    {
        ++scroll_data.horizontal;
    }

    if (event.key.code == bindings.ScrollRight)
    {
        --scroll_data.horizontal;
    }

    if (event.key.code == bindings.ScrollUp)
    {
        ++scroll_data.vertical;
    }

    if (event.key.code == bindings.ScrollDown)
    {
        --scroll_data.vertical;
    }
}



} // client
