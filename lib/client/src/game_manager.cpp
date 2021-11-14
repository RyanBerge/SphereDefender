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

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

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

    if (ServerSocket.connect(sf::IpAddress(ip), network::SERVER_PORT, sf::seconds(5)) != sf::Socket::Status::Done)
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
                // TODO: Route and handle this
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
                                // TODO: Handle this
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
                        // TODO: Handle this
                    }
                    break;
                }
            }
        }
        break;
        case ServerMessage::Code::OwnerLeft:
        {
            // TODO: Destroy everything and return to Main Menu
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
                            // TODO: Handle this
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
                    // TODO: Handle this
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
            if (State == GameState::MainMenu && MainMenu.CurrentMenu == MainMenu::MenuType::LoadingScreen)
            {
                MainMenu.Unload();
                State = GameState::Game;
                // Game.Start();
            }
        }
        break;
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
    float desired_ratio = DEFAULT_RATIO.x / DEFAULT_RATIO.y;
    float current_ratio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);

    sf::View view(sf::FloatRect(0, 0, DEFAULT_RATIO.x, DEFAULT_RATIO.y));
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
