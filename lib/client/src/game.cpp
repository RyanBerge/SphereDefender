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

Game::Game() : WorldView(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y)) { }

void Game::Update(sf::Time elapsed)
{
    if (loaded)
    {
        local_player.Update(elapsed);
        for (auto& avatar : avatars)
        {
            avatar.second.Update(elapsed);
        }

        if (current_zoom != target_zoom)
        {
            current_zoom += zoom_speed * elapsed.asSeconds();
            zoom_speed = (target_zoom - current_zoom) * 4;

            if (std::abs(current_zoom - target_zoom) < 0.001)
            {
                current_zoom = target_zoom;
            }

            WorldView.setSize(Settings::GetInstance().WindowResolution * current_zoom);
        }

//        if (GameManager::GetInstance().Window.hasFocus() && !menu_open)
//        {
//            updateScroll(elapsed);
//        }
        WorldView.setCenter(local_player.GetPosition());
    }
}

void Game::Draw()
{
    if (loaded)
    {
        sf::View old_view = GameManager::GetInstance().Window.getView();
        WorldView.setViewport(old_view.getViewport());
        GameManager::GetInstance().Window.setView(WorldView);

        region_map.Draw();
        local_player.Draw();
        for (auto& avatar : avatars)
        {
            avatar.second.Draw();
        }

        for (auto& enemy : enemies)
        {
            enemy.second.Draw();
        }

        GameManager::GetInstance().Window.setView(old_view);

        gui.Draw();

        if (menu_open)
        {
            Settings::GetInstance().Draw();
        }
    }
}

void Game::Load(network::PlayerData local, std::vector<network::PlayerData> other_players)
{
    loaded = false;
    std::thread loading_thread(&Game::asyncLoad, this, local, other_players);
    loading_thread.detach();
}

void Game::asyncLoad(network::PlayerData local, std::vector<network::PlayerData> other_players)
{
    std::cout << "Async load started..." << std::endl;

    event_id_map[sf::Event::EventType::MouseMoved] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&Game::onMouseMove, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Game::onMouseDown, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Game::onMouseUp, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::TextEntered] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&Game::onTextEntered, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::KeyPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyPressed, std::bind(&Game::onKeyPressed, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::KeyReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyReleased, std::bind(&Game::onKeyReleased, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseWheelScrolled] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseWheelScrolled, std::bind(&Game::onMouseWheel, this, std::placeholders::_1));

    menu_open = false;
    zoom_factor = 0;
    current_zoom = 1;
    target_zoom = current_zoom;
    zoom_speed = 0;

    avatars.clear();
    enemies.clear();

    local_player = Player();

    region_map.Load();
    gui.Load();
    local_player.Load(local);

    for (auto& player : other_players)
    {
        avatars[player.id] = Avatar(sf::Color(180, 115, 150), player);
    }

    WorldView = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));

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

    // TODO: These load/unload functions probably need to be protected by mutexes in case the host leaves when the game is loading
    region_map.Unload();
    gui.Unload();
    local_player.Unload();
}

void Game::Start(sf::Vector2f spawn_position)
{
    local_player.SetPosition(spawn_position);
    WorldView.setCenter(spawn_position);
}

void Game::InitializeRegion(network::ConvoyData convoy_data, std::vector<network::EnemyData> enemy_list)
{
    for (auto& enemy : enemy_list)
    {
        enemies[enemy.id] = Enemy();
        enemies[enemy.id].UpdateData(enemy);
    }

    region_map.InitializeRegion(convoy_data);
}

int Game::GetPlayerCount()
{
    return avatars.size() + 1;
}

void Game::UpdatePlayerStates(std::vector<network::PlayerData> player_list)
{
    for (auto& player : player_list)
    {
        if (player.id == local_player.Avatar.Data.id)
        {
            local_player.SetPosition(player.position);
            local_player.Avatar.UpdateHealth(player.health);
            gui.UpdateHealth(player.health);
        }
        else
        {
            avatars[player.id].SetPosition(player.position);
            avatars[player.id].UpdateHealth(player.health);
        }
    }
}

void Game::UpdateEnemies(std::vector<network::EnemyData> enemy_list)
{
    for (auto& enemy : enemy_list)
    {
        enemies[enemy.id].UpdateData(enemy);
        if (enemies[enemy.id].GetData().health == 0)
        {
            enemies.erase(enemy.id);
        }
    }
}

void Game::StartAction(uint16_t player_id, network::PlayerAction action)
{
    if (local_player.Avatar.Data.id == player_id)
    {
    }
    else
    {
        if (action.start_attack)
        {
            avatars[player_id].StartAttack(action.attack_angle);
        }
    }
}

void Game::StartEnemyAction(uint16_t enemy_id, network::EnemyAction action)
{
    enemies[enemy_id].StartAction(action);
}

void Game::RemovePlayer(uint16_t player_id)
{
    // TODO: Thread-safety
    cout << avatars[player_id].Data.name << " disconnected." << endl;
    avatars.erase(player_id);
}

void Game::updateScroll(sf::Time elapsed)
{
    sf::Vector2f mouse_coords = GameManager::GetInstance().Window.mapPixelToCoords(sf::Mouse::getPosition() - GameManager::GetInstance().Window.getPosition(), gui.GuiView);

    sf::Vector2i scroll_factor{0, 0};
    sf::Vector2f resolution = Settings::GetInstance().WindowResolution;

    int edge_threshold = 15;

    if (mouse_coords.x < edge_threshold)
    {
        scroll_factor.x = -1;
    }
    else if (mouse_coords.x > resolution.x - edge_threshold)
    {
        scroll_factor.x = 1;
    }

    if (mouse_coords.y < edge_threshold)
    {
        scroll_factor.y = -1;
    }
    else if (mouse_coords.y > resolution.y - edge_threshold)
    {
        scroll_factor.y = 1;
    }

    WorldView.move(sf::Vector2f(scroll_factor * Settings::GetInstance().ScrollSpeed) * current_zoom * elapsed.asSeconds());
}

// TODO: Ensure that these are only updated for elements that can be controlled at that time
void Game::onMouseMove(sf::Event event)
{
    if (loaded)
    {
        gui.OnMouseMove(event.mouseMove);
        local_player.OnMouseMove(event.mouseMove);
    }
}

void Game::onMouseDown(sf::Event event)
{
    if (loaded)
    {
        gui.OnMouseDown(event.mouseButton);
        local_player.OnMouseDown(event.mouseButton);
    }
}

void Game::onMouseUp(sf::Event event)
{
    if (loaded)
    {
        gui.OnMouseUp(event.mouseButton);
        local_player.OnMouseUp(event.mouseButton);
    }
}

void Game::onTextEntered(sf::Event event)
{
    if (loaded)
    {
        gui.OnTextEntered(event.text);
        local_player.OnTextEntered(event.text);
    }
}

void Game::onKeyPressed(sf::Event event)
{
    if (loaded)
    {
        if (!menu_open) {
            local_player.OnKeyPressed(event.key);
        }

        if (event.key.code == Settings::GetInstance().Bindings.Pause)
        {
            gui.InMenus = !gui.InMenus;
        }
    }
}

void Game::onKeyReleased(sf::Event event)
{
    if (loaded)
    {
        local_player.OnKeyReleased(event.key);
    }
}

void Game::onMouseWheel(sf::Event event)
{
    if (loaded)
    {
        Settings& settings = Settings::GetInstance();

        zoom_factor -= event.mouseWheelScroll.delta;
        if (zoom_factor < settings.MinZoomFactor)
        {
            zoom_factor = settings.MinZoomFactor;
        }
        else if (zoom_factor > settings.MaxZoomFactor)
        {
            zoom_factor = settings.MaxZoomFactor;
        }

        target_zoom = 1 + static_cast<float>(zoom_factor) / 10;
        zoom_speed = target_zoom - current_zoom;
    }
}

} // client
