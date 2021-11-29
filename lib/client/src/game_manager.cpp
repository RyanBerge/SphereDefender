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
#include <SFML/Network/IpAddress.hpp>
#include <functional>
#include <iostream>
#include "game_manager.h"
#include "event_handler.h"
#include "messaging.h"
#include "settings.h"

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

    Settings::GetInstance();
    Window.create(sf::VideoMode(1200, 800), "Sphere Defender");

    sf::Event event;
    event.size.width = 1200;
    event.size.height = 800;
    onResizeWindow(event);

    Window.setKeyRepeatEnabled(false);
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
                Game.Update(elapsed);
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
                Game.Draw();
            }
            break;
        }

        Window.display();

        sf::Event event;
        while (Window.pollEvent(event))
        {
            event_handler.AddEvent(event);
        }

        checkMessages();
    }
}

void GameManager::ExitGame()
{
    Window.close();
    running = false;
}

bool GameManager::ConnectToServer(std::string ip)
{
    ServerSocket.setBlocking(true);

    if (ServerSocket.connect(sf::IpAddress(ip), Settings::GetInstance().ServerSettings.ServerPort, sf::seconds(5)) != sf::Socket::Status::Done)
    {
        cerr << "Server at " << ip << " could not be reached." << endl;
        return false;
    }

    ServerSocket.setBlocking(false);
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
                    ClientMessage::LeaveGame(ServerSocket);
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
            ClientMessage::LeaveGame(ServerSocket);
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
    ServerSocket.disconnect();
    server_connected = false;
}

void GameManager::checkMessages()
{
    if (!server_connected)
    {
        return;
    }

    ServerMessage::Code code;
    bool success = ServerMessage::PollForCode(ServerSocket, code);
    if (!success)
    {
        cerr << "You were disconnected unexpectedly." << endl;
        handleDisconnected();
        return;
    }

    switch (code)
    {
        case ServerMessage::Code::None:
        {
            return;
        }
        break;
        case ServerMessage::Code::Error:
        {
            cerr << "Error codes not yet implemented." << endl;
        }
        break;
        case ServerMessage::Code::PlayerId:
        {
            uint16_t player_id;
            if (ServerMessage::DecodePlayerId(ServerSocket, player_id))
            {
                if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::Lobby)
                {
                    MainMenu.Lobby.AssignId(player_id);
                }
                else
                {
                    cerr << "PlayerId message received when client wasn't in a lobby." << endl;
                }
            }
        }
        break;
        case ServerMessage::Code::PlayerJoined:
        {
            network::PlayerData data;
            if (ServerMessage::DecodePlayerJoined(ServerSocket, data))
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
            if (ServerMessage::DecodePlayerLeft(ServerSocket, player_id))
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
            if (ServerMessage::DecodePlayersInLobby(ServerSocket, player_id, player_list))
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
            if (ServerMessage::DecodeAllPlayersLoaded(ServerSocket, spawn_position))
            {
                if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::LoadingScreen)
                {
                    MainMenu.Unload();
                    State = GameState::Game;
                    Game.Start(spawn_position);
                }
            }
        }
        break;
        case ServerMessage::Code::PlayerStates:
        {
            std::vector<network::PlayerData> player_list;
            if (ServerMessage::DecodePlayerStates(ServerSocket, player_list))
            {
                Game.UpdatePlayerStates(player_list);
            }
        }
        break;
        case ServerMessage::Code::StartAction:
        {
            uint16_t player_id;
            network::PlayerAction action;
            if (ServerMessage::DecodeStartAction(ServerSocket, player_id, action))
            {
                Game.StartAction(player_id, action);
            }
        }
        break;
        case ServerMessage::Code::RegionInfo:
        {
            std::vector<network::EnemyData> enemy_list;
            if (ServerMessage::DecodeRegionInfo(ServerSocket, enemy_list))
            {
                Game.InitializeRegion(enemy_list);
            }
        }
        break;
        case ServerMessage::Code::EnemyUpdate:
        {
            std::vector<network::EnemyData> enemy_list;
            if (ServerMessage::DecodeEnemyUpdate(ServerSocket, enemy_list))
            {
                Game.UpdateEnemies(enemy_list);
            }
        }
        break;
        default:
        {
            cerr << "Unrecognized code: " << static_cast<int>(code) << endl;
        }
    }
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
        // cout << "window is too wide" << endl;
        viewport.width = desired_ratio / current_ratio;
        viewport.left = (1 - viewport.width) / 2;
    }
    else if (current_ratio < desired_ratio)
    {
        // cout << "window is too tall" << endl;
        viewport.height = current_ratio / desired_ratio;
        viewport.top = (1 - viewport.height) / 2;
    }

    view.setViewport(viewport);
    Window.setView(view);
}

} // client
