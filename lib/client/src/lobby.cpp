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
#include "util.h"
#include "messaging.h"
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Lobby::Lobby()
{
    leave_button = CursorButton("LeaveGameButton.png");
    leave_button.SetPosition(472, 650);
    leave_button.RegisterLeftMouseDown(std::bind(&Lobby::LeaveLobby, this));

    start_button = CursorButton("SplashStart.png");
    start_button.SetPosition(472, 550);
    start_button.RegisterLeftMouseDown(std::bind(&Lobby::StartGame, this));

    font = util::AllocFont("Vera.ttf");

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

    local_player = PlayerState();
    player_display_list.clear();

    // TODO: Unload UI elements as well?
}

bool Lobby::Create(std::string player_name)
{
    // launch server
    cerr << "Launching server..." << endl;
    std::system("start Server.exe");

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

    GameManager::GetInstance().Window.draw(local_player.lobby_display_text);
    for (auto& display_player : player_display_list)
    {
        GameManager::GetInstance().Window.draw(display_player.lobby_display_text);
    }
}

void Lobby::StartGame()
{
    if (owner)
    {
        ClientMessage::StartGame(ServerSocket);
    }

    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::LoadingScreen;

    GameManager::GetInstance().Game.Load(local_player, player_display_list);

    // TODO: Give the Game the player information
    Unload();
}

void Lobby::AssignId(uint16_t id)
{
    local_player.data.id = id;
    local_player.lobby_display_text.setScale(1.2, 1.2);
    local_player.lobby_display_text.setFont(*font);
    local_player.lobby_display_text.setString(local_player.data.name);

    updatePlayerPositions();
}

void Lobby::AddPlayer(network::PlayerData player_data)
{
    PlayerState lobby_player;
    lobby_player.data = player_data;
    lobby_player.lobby_display_text.setFont(*font);
    lobby_player.lobby_display_text.setString(player_data.name);

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

    local_player.lobby_display_text.setPosition(sf::Vector2f(50, 50 + offset));
    offset += 45;

    for (auto& player : player_display_list)
    {
        player.lobby_display_text.setPosition(sf::Vector2f(50, 50 + offset));
        offset += 35;
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
