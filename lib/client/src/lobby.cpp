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
//#include "state_manager.h"
#include "game_manager.h"
//#include "player.h"
#include "util.h"
#include "messaging.h"
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

//Lobby* Lobby::lobby_instance = nullptr;

Lobby::Lobby()
{
    leave_button = CursorButton("LeaveGameButton.png");
    leave_button.SetPosition(472, 650);
    leave_button.RegisterLeftMouseDown(std::bind(&Lobby::LeaveLobby, this));

    start_button = CursorButton("SplashStart.png");
    start_button.SetPosition(472, 550);
    start_button.RegisterLeftMouseDown(std::bind(&Lobby::StartGame, this));

    font = util::AllocFont("assets/Vera.ttf");

    if (font == nullptr)
    {
        std::cerr << "Lobby: There was an error loading the font." << std::endl;
    }
}

void Lobby::initializeMenu()
{
    //lobby_instance = this;
    mouse_down_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Lobby::onMouseDown, this, std::placeholders::_1));
    mouse_up_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Lobby::onMouseUp, this, std::placeholders::_1));
}

bool Lobby::Create(std::string player_name)
{
    // launch server
    cerr << "Launching server..." << endl;
    std::system("start build/bin/Server.exe");

    if (!GameManager::GetInstance().ConnectToServer("127.0.0.1"))
    {
        return false;
    }

    ClientMessage::InitLobby(ServerSocket, player_name);
//    Player::state.name = player_name;

    owner = true;
    local_player.name = player_name;

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
//    Player::state.name = player_name;

    owner = false;
    local_player.name = player_name;

    initializeMenu();

    return true;
}
/*
void Lobby::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    leave_button.Update(elapsed, window);
    if (owner)
    {
        start_button.Update(elapsed, window);
    }
}
*/
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

void Lobby::Unload()
{
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseButtonPressed, mouse_down_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseButtonReleased, mouse_up_id);
    local_player = LobbyPlayer{};
    player_display_list.clear();

    // TODO: Unload UI elements as well?
}

void Lobby::StartGame()
{
    if (owner)
    {
        ClientMessage::StartGame(ServerSocket);
    }

    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::LoadingScreen;
    GameManager::GetInstance().Game.Load();

    // TODO: Give the Game the player information
    Unload();
}

void Lobby::AssignId(uint16_t id)
{
    local_player.id = id;
    local_player.display_text.setScale(1.2, 1.2);
    local_player.display_text.setFont(*font);
    local_player.display_text.setString(local_player.name);

    updatePlayerPositions();
}

void Lobby::AddPlayer(network::PlayerData player)
{
    LobbyPlayer lobby_player;
    lobby_player.id = player.id;
    lobby_player.name = player.name;
    lobby_player.display_text.setFont(*font);
    lobby_player.display_text.setString(player.name);

    player_display_list.push_back(lobby_player);

    updatePlayerPositions();
}

void Lobby::RemovePlayer(uint16_t player_id)
{
    for (auto iterator = player_display_list.begin(); iterator != player_display_list.end(); ++iterator)
    {
        if (iterator->id == player_id)
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

    local_player.display_text.setPosition(sf::Vector2f(50, 50 + offset));
    offset += 45;

    for (auto& player : player_display_list)
    {
        player.display_text.setPosition(sf::Vector2f(50, 50 + offset));
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
