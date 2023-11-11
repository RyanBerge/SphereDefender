/**************************************************************************************************
 *  File:       game_manager.cpp
 *  Class:      GameManager
 *
 *  Purpose:    GameManager is the top-level state manager for the client
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <SFML/Network/IpAddress.hpp>
#include "SFML/System/Sleep.hpp"
#include <functional>
#include <iostream>
#include "game_manager.h"
#include "event_handler.h"
#include "messaging.h"
#include "settings.h"
#include "resources.h"
#include "debug_overrides.h"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

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

    Settings settings = Settings::GetInstance();
    sf::ContextSettings render_context_settings;
    render_context_settings.antialiasingLevel = settings.AntiAliasing;

    sf::Vector2u window_size{1280, 720};
    resources::GetWindow().create(sf::VideoMode(window_size.x, window_size.y), "Sphere Defender",sf::Style::Default, render_context_settings);

    sf::Event resize_event;
    resize_event.size.width = window_size.x;
    resize_event.size.height = window_size.y;
    onResizeWindow(resize_event);

    resources::GetWindow().setKeyRepeatEnabled(false);
    sf::Clock clock;

    running = true;

#ifndef NDEBUG
    if (debug::StartInGame.override && debug::StartInGame.value)
    {
        resources::GetWindow().clear(sf::Color::Black);
        resources::GetWindow().display();
        MainMenu.Lobby.Create("Ryan");
        MainMenu.CurrentMenu = MainMenu::MenuType::Lobby;
        checkMessages();
        MainMenu.Lobby.StartGame();
    }
#endif

    while (running && resources::GetWindow().isOpen())
    {
        sf::Clock loop_timer;
        sf::Time elapsed = clock.restart();

        event_handler.RunCallbacks();

        // TODO: Will we need to update one of these even outside of its state?
        switch (State)
        {
            case GameState::MainMenu:
            {
                MainMenu.Update(elapsed);
            }
            break;
            case GameState::Game:
            {
                Game.Update(elapsed);
            }
            break;
        }

        resources::GetWindow().clear(sf::Color::Black);

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
                Game.Draw();
            }
            break;
        }

        resources::GetWindow().display();

        sf::Event event;
        int num = 0;
        while (resources::GetWindow().pollEvent(event))
        {
            ++num;
            event_handler.AddEvent(event);
        }

        checkMessages();

        static bool slow_loop = false;
        if (loop_timer.getElapsedTime().asSeconds() > 1 / 120.0f)
        {
            if (slow_loop)
            {
                cerr << "Game loop took longer than desired twice in a row: " << loop_timer.getElapsedTime().asMilliseconds() << " milliseconds\n";
            }
            slow_loop = true;
        }
        else
        {
            slow_loop = false;
        }
    }
}

void GameManager::ExitGame()
{
    resources::GetWindow().close();
    running = false;
}

bool GameManager::ConnectToServer(std::string ip)
{
    resources::GetServerSocket().setBlocking(true);

    if (resources::GetServerSocket().connect(sf::IpAddress(ip), Settings::GetInstance().ServerSettings.ServerPort, sf::seconds(5)) != sf::Socket::Status::Done)
    {
        cerr << "Server at " << ip << " could not be reached." << endl;
        return false;
    }

    resources::GetServerSocket().setBlocking(false);
    server_connected = true;

    return true;
}

void GameManager::Reset()
{
    switch (State)
    {
        case GameState::MainMenu:
        {
            switch (MainMenu.CurrentMenu)
            {
                case MainMenu::MenuType::Lobby:
                case MainMenu::MenuType::LoadingScreen:
                {
                    ClientMessage::LeaveGame(resources::GetServerSocket());
                    DisconnectFromServer();
                    MainMenu.CurrentMenu = MainMenu::MenuType::Main;
                }
                default:
                {

                }
            }
        }
        break;
        case GameState::Game:
        {
            ClientMessage::LeaveGame(resources::GetServerSocket());
            DisconnectFromServer();
            Game.Unload();
            State = GameState::MainMenu;
            MainMenu.CurrentMenu = MainMenu::MenuType::Main;
            MainMenu.Load();
        }
        break;
    }
}

void GameManager::DisconnectFromServer()
{
    resources::GetServerSocket().disconnect();
    server_connected = false;
}

void GameManager::checkMessages()
{
    if (!server_connected)
    {
        return;
    }

    ServerMessage::Code code = ServerMessage::Code::None;
    int num_messages = 0;

    do
    {
        bool success = ServerMessage::PollForCode(resources::GetServerSocket(), code);
        if (!success)
        {
            cerr << "You were disconnected unexpectedly." << endl;
            handleDisconnected();
            return;
        }

        ++num_messages;

        switch (code)
        {
            case ServerMessage::Code::None: { }
            break;
            case ServerMessage::Code::Error:
            {
                cerr << "Error codes not yet implemented." << endl;
            }
            break;
            case ServerMessage::Code::PlayerId:
            {
                uint16_t player_id;
                if (ServerMessage::DecodePlayerId(resources::GetServerSocket(), player_id))
                {
                    if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::Lobby)
                    {
                        MainMenu.Lobby.AssignId(player_id);
                    }
                    else
                    {
                        cerr << "PlayerId message received when client wasn't in a lobby." << endl;
                        cerr << (int)State << ", " << (int)MainMenu.CurrentMenu << endl;
                    }
                }
            }
            break;
            case ServerMessage::Code::PlayerJoined:
            {
                network::PlayerData data{};
                if (ServerMessage::DecodePlayerJoined(resources::GetServerSocket(), data))
                {
                    if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::Lobby)
                    {
                        MainMenu.Lobby.AddPlayer(data);
                    }
                    else
                    {
                        cerr << "PlayerJoined message received when client wasn't in a lobby." << endl;
                    }
                }
            }
            break;
            case ServerMessage::Code::PlayerLeft:
            {
                uint16_t player_id;
                if (ServerMessage::DecodePlayerLeft(resources::GetServerSocket(), player_id))
                {
                    switch (State)
                    {
                        case GameState::MainMenu:
                        {
                            switch (MainMenu.CurrentMenu)
                            {
                                case MainMenu::MenuType::Lobby:
                                {
                                    MainMenu.Lobby.RemovePlayer(player_id);
                                }
                                break;
                                case MainMenu::MenuType::LoadingScreen:
                                {
                                    Game.RemovePlayer(player_id);
                                }
                                break;
                                default:
                                {
                                    cerr << "PlayerLeft message received when the client wasn't in a server." << endl;
                                }
                            }
                        }
                        break;
                        case GameState::Game:
                        {
                            Game.RemovePlayer(player_id);
                        }
                        break;
                    }
                }
            }
            break;
            case ServerMessage::Code::OwnerLeft:
            {
                cout << "The host left the game and the session was terminated." << endl;
                switch (State)
                {
                    case GameState::MainMenu:
                    {
                        switch (MainMenu.CurrentMenu)
                        {
                            case MainMenu::MenuType::Lobby:
                            {
                                MainMenu.Lobby.LeaveLobby();
                            }
                            break;
                            case MainMenu::MenuType::LoadingScreen:
                            {
                                Reset();
                            }
                            break;
                            default:
                            {
                                cerr << "Somehow a player left when the client wasn't in a server." << endl;
                            }
                        }
                    }
                    break;
                    case GameState::Game:
                    {
                        Reset();
                    }
                    break;
                }
            }
            break;
            case ServerMessage::Code::PlayersInLobby:
            {
                uint16_t player_id;
                std::vector<network::PlayerData> player_list;
                if (ServerMessage::DecodePlayersInLobby(resources::GetServerSocket(), player_id, player_list))
                {
                    if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::Lobby)
                    {
                        MainMenu.Lobby.AssignId(player_id);
                        for (auto& data : player_list)
                        {
                            MainMenu.Lobby.AddPlayer(data);
                        }
                    }
                }
            }
            break;
            case ServerMessage::Code::ChangePlayerProperty:
            {
                uint16_t player_id;
                network::PlayerProperties properties;
                if (ServerMessage::DecodeChangePlayerProperty(resources::GetServerSocket(), player_id, properties))
                {
                    if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::Lobby)
                    {
                        MainMenu.Lobby.SetPlayerProperties(player_id, properties);
                    }
                }
            }
            break;
            case ServerMessage::Code::StartGame:
            {
                if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::Lobby)
                {
                    MainMenu.Lobby.StartGame();
                }
            }
            break;
            case ServerMessage::Code::AllPlayersLoaded:
            {
                sf::Vector2f spawn_position;
                if (ServerMessage::DecodeAllPlayersLoaded(resources::GetServerSocket(), spawn_position))
                {
                    if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::LoadingScreen)
                    {
                        MainMenu.Unload();
                        State = GameState::Game;
                        Game.Start(spawn_position);
                    }
                    else if (State == GameState::Game)
                    {
                        Game.EnterRegion(spawn_position);
                    }
                }
            }
            break;
            case ServerMessage::Code::SetZone:
            {
                definitions::Zone zone;
                if (ServerMessage::DecodeSetZone(resources::GetServerSocket(), zone))
                {
                    if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::LoadingScreen)
                    {
                        Game.SetZone(zone);
                    }
                }
            }
            break;
            case ServerMessage::Code::SetGuiPause:
            {
                bool paused;
                network::GuiType gui_type;
                if (ServerMessage::DecodeSetGuiPause(resources::GetServerSocket(), paused, gui_type))
                {
                    if (State == GameState::Game)
                    {
                        Game.SetPaused(paused, gui_type);
                    }
                }
            }
            break;
            case ServerMessage::Code::PlayerStartAction:
            {
                uint16_t player_id;
                network::PlayerAction action;
                if (ServerMessage::DecodePlayerStartAction(resources::GetServerSocket(), player_id, action))
                {
                    Game.StartAction(player_id, action);
                }
            }
            break;
            case ServerMessage::Code::EnemyChangeAction:
            {
                uint16_t enemy_id;
                network::EnemyAnimation action;
                if (ServerMessage::DecodeEnemyChangeAction(resources::GetServerSocket(), enemy_id, action))
                {
                    Game.ChangeEnemyAction(enemy_id, action);
                }
            }
            break;
            case ServerMessage::Code::ChangeItem:
            {
                definitions::ItemType item;
                if (ServerMessage::DecodeChangeItem(resources::GetServerSocket(), item))
                {
                    Game.ChangeItem(item);
                }
            }
            break;
            case ServerMessage::Code::PlayerStates:
            {
                std::vector<network::PlayerData> player_list;
                if (ServerMessage::DecodePlayerStates(resources::GetServerSocket(), player_list))
                {
                    Game.UpdatePlayerStates(player_list);
                }
            }
            break;
            case ServerMessage::Code::AddEnemy:
            {
                uint16_t enemy_id;
                definitions::EntityType type;
                if (ServerMessage::DecodeAddEnemy(resources::GetServerSocket(), enemy_id, type))
                {
                    Game.AddEnemy(enemy_id, type);
                }
            }
            break;
            case ServerMessage::Code::EnemyUpdate:
            {
                std::vector<network::EnemyData> enemy_list;
                if (ServerMessage::DecodeEnemyUpdate(resources::GetServerSocket(), enemy_list))
                {
                    Game.UpdateEnemies(enemy_list);
                }
            }
            break;
            case ServerMessage::Code::ProjectileUpdate:
            {
                std::vector<network::ProjectileData> projectile_list;
                if (ServerMessage::DecodeProjectileUpdate(resources::GetServerSocket(), projectile_list))
                {
                    Game.UpdateProjectiles(projectile_list);
                }
            }
            break;
            case ServerMessage::Code::BatteryUpdate:
            {
                float battery_level;
                if (ServerMessage::DecodeBatteryUpdate(resources::GetServerSocket(), battery_level))
                {
                    Game.UpdateBattery(battery_level);
                }
            }
            break;
            case ServerMessage::Code::ChangeRegion:
            {
                uint16_t region_id;
                if (ServerMessage::DecodeChangeRegion(resources::GetServerSocket(), region_id))
                {
                    Game.ChangeRegion(region_id);
                }
            }
            break;
            case ServerMessage::Code::UpdateStash:
            {
                std::array<definitions::ItemType, 24> items;
                if (ServerMessage::DecodeUpdateStash(resources::GetServerSocket(), items))
                {
                    Game.UpdateStash(items);
                }
            }
            break;
            case ServerMessage::Code::GatherPlayers:
            {
                uint16_t player_id;
                bool start;
                if (ServerMessage::DecodeGatherPlayers(resources::GetServerSocket(), player_id, start))
                {
                    if (State == GameState::Game)
                    {
                        Game.DisplayGatherPlayers(player_id, start);
                    }
                    else
                    {
                        cerr << "Code received during an inappropriate game state: " << static_cast<int>(code) << "\n";
                    }
                }
            }
            break;
            case ServerMessage::Code::CastVote:
            {
                uint16_t player_id;
                uint8_t vote;
                bool confirm;
                if (ServerMessage::DecodeCastVote(resources::GetServerSocket(), player_id, vote, confirm))
                {
                    if (State == GameState::Game)
                    {
                        Game.DisplayVote(player_id, vote, confirm);
                    }
                    else
                    {
                        cerr << "Code received during an inappropriate game state: " << static_cast<int>(code) << "\n";
                    }
                }
            }
            break;
            case ServerMessage::Code::SetMenuEvent:
            {
                uint16_t event_id;
                if (ServerMessage::DecodeSetMenuEvent(resources::GetServerSocket(), event_id))
                {
                    if (State == GameState::Game)
                    {
                        Game.SetMenuEvent(event_id);
                    }
                    else
                    {
                        cerr << "Code received during an inappropriate game state: " << static_cast<int>(code) << "\n";
                    }
                }
            }
            break;
            case ServerMessage::Code::AdvanceMenuEvent:
            {
                uint16_t advance_value;
                bool finish;
                if (ServerMessage::DecodeAdvanceMenuEvent(resources::GetServerSocket(), advance_value, finish))
                {
                    if (State == GameState::Game)
                    {
                        Game.AdvanceMenuEvent(advance_value, finish);
                    }
                    else
                    {
                        cerr << "Code received during an inappropriate game state: " << static_cast<int>(code) << "\n";
                    }
                }
            }
            break;
            case ServerMessage::Code::DisplayPath:
            {
                std::vector<sf::Vector2f> graph;
                std::vector<sf::Vector2f> path;
                if (ServerMessage::DecodeDisplayPath(resources::GetServerSocket(), graph, path))
                {
                    if (State == GameState::Game)
                    {
                        Game.DisplayDebugPath(graph, path);
                    }
                    else
                    {
                        cerr << "Code received during an inappropriate game state: " << static_cast<int>(code) << "\n";
                    }
                }
            }
            break;
            default:
            {
                cerr << "Unrecognized code: " << static_cast<int>(code) << endl;
            }
        }

        //cout << (int)code << endl;

        if (num_messages > 20)
        {
            break;
        }
    }
    while (code != ServerMessage::Code::None && server_connected);
}

void GameManager::handleDisconnected()
{
    switch (State)
    {
        case GameState::MainMenu:
        {
            switch (MainMenu.CurrentMenu)
            {
                case MainMenu::MenuType::Lobby:
                {
                    MainMenu.Lobby.Unload();
                    MainMenu.CurrentMenu = MainMenu::MenuType::Main;
                }
                break;
                case MainMenu::MenuType::LoadingScreen:
                {
                    // TODO: Handle disconnects on load
                }
                break;
                default:
                {
                    cerr << "What exactly is going on here?" << endl;
                    MainMenu.CurrentMenu = MainMenu::MenuType::Main;
                }
                break;
            }
        }
        break;
        case GameState::Game:
        {
            // TODO: handle this
        }
        break;
    }

    server_connected = false;
}

void GameManager::onCloseWindow(sf::Event event)
{
    (void)event;
    ExitGame();
}

void GameManager::onResizeWindow(sf::Event event)
{
    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    float desired_ratio = window_resolution.x / window_resolution.y;
    float current_ratio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);

    sf::View view(sf::FloatRect(0, 0, window_resolution.x, window_resolution.y));
    sf::FloatRect viewport(0, 0, 1, 1);

    if (current_ratio > desired_ratio)
    {
        viewport.width = desired_ratio / current_ratio;
        viewport.left = (1 - viewport.width) / 2;
    }
    else if (current_ratio < desired_ratio)
    {
        viewport.height = current_ratio / desired_ratio;
        viewport.top = (1 - viewport.height) / 2;
    }

    view.setViewport(viewport);
    resources::GetWindow().setView(view);
}

} // client
