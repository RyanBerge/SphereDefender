/**************************************************************************************************
 *  File:       lobby.h
 *  Class:      MainMenu
 *
 *  Purpose:    The menu that handles game lobbies
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "lobby.h"
#include "event_handler.h"
#include "game_manager.h"
#include "resources.h"
#include "messaging.h"
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Lobby::Lobby()
{
    leave_button = CursorButton("LeaveGameButton.png");
    leave_button.SetPosition(542, 1000);
    leave_button.RegisterLeftMouseDown(std::bind(&Lobby::LeaveLobby, this));

    start_button = CursorButton("StartServerButton.png");
    start_button.SetPosition(795, 850);
    start_button.RegisterLeftMouseDown(std::bind(&Lobby::StartGame, this));

    font = resources::AllocFont("Vera.ttf");

    if (font == nullptr)
    {
        std::cerr << "Lobby: There was an error loading the font." << std::endl;
    }
}

void Lobby::initializeMenu()
{
    //lobby_instance = this;
    event_id_map[sf::Event::EventType::MouseButtonPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Lobby::onMouseDown, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Lobby::onMouseUp, this, std::placeholders::_1));
}

void Lobby::Unload()
{
    for (auto& event : event_id_map)
    {
        EventHandler::GetInstance().UnregisterCallback(event.first, event.second);
    }

    local_player = LobbyPlayer{};
    player_display_list.clear();

    // TODO: Unload UI elements as well?
}

bool Lobby::Create(std::string player_name)
{
    // launch server
    cerr << "Launching server..." << endl;

#if __linux__
    std::system("start Server");
#else
    std::system("start Server.exe");
#endif

    if (!GameManager::GetInstance().ConnectToServer("127.0.0.1"))
    {
        return false;
    }

    ClientMessage::InitLobby(ServerSocket, player_name);

    owner = true;
    local_player.data.name = player_name;

    initializeMenu();

    return true;
}

bool Lobby::Join(std::string player_name, std::string ip)
{
    if (!GameManager::GetInstance().ConnectToServer(ip))
    {
        return false;
    }

    ClientMessage::JoinLobby(ServerSocket, player_name);

    owner = false;
    local_player.data.name = player_name;

    initializeMenu();

    return true;
}

void Lobby::Draw()
{
    leave_button.Draw();
    if (owner)
    {
        start_button.Draw();
    }

    GameManager::GetInstance().Window.draw(local_player.display_text);
    for (auto& display_player : player_display_list)
    {
        GameManager::GetInstance().Window.draw(display_player.display_text);
    }
}

void Lobby::StartGame()
{
    if (owner)
    {
        ClientMessage::StartGame(ServerSocket);
    }

    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::LoadingScreen;

    std::vector<network::PlayerData> data;
    for (auto& player : player_display_list)
    {
        data.push_back(player.data);
    }

    GameManager::GetInstance().Game.Load(local_player.data, data);

    // TODO: Give the Game the player information
    Unload();
}

void Lobby::AssignId(uint16_t id)
{
    local_player.data.id = id;
    local_player.display_text.setFont(*font);
    local_player.display_text.setString(local_player.data.name);
    local_player.display_text.setCharacterSize(90);

    updatePlayerPositions();
}

void Lobby::AddPlayer(network::PlayerData player_data)
{
    LobbyPlayer lobby_player;
    lobby_player.data = player_data;
    lobby_player.display_text.setFont(*font);
    lobby_player.display_text.setString(player_data.name);
    lobby_player.display_text.setCharacterSize(75);

    player_display_list.push_back(lobby_player);

    updatePlayerPositions();
}

void Lobby::RemovePlayer(uint16_t player_id)
{
    for (auto iterator = player_display_list.begin(); iterator != player_display_list.end(); ++iterator)
    {
        if (iterator->data.id == player_id)
        {
            player_display_list.erase(iterator);
            break;
        }
    }

    updatePlayerPositions();
}

void Lobby::LeaveLobby()
{
    Unload();
    ClientMessage::LeaveGame(ServerSocket);
    GameManager::GetInstance().DisconnectFromServer();
    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::Main;
}

void Lobby::updatePlayerPositions()
{
    int offset = 0;

    local_player.display_text.setPosition(sf::Vector2f(75, 75 + offset));
    offset += 100;

    for (auto& player : player_display_list)
    {
        player.display_text.setPosition(sf::Vector2f(75, 75 + offset));
        offset += 75;
    }
}

void Lobby::onMouseDown(sf::Event event)
{
    leave_button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
    start_button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
}

void Lobby::onMouseUp(sf::Event event)
{
    leave_button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
    start_button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
}

} // client
