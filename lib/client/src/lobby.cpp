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
//#include "messaging.h"
//#include "state_manager.h"
#include "game_manager.h"
//#include "player.h"
#include "util.h"
#include "messaging.h"
#include <iostream>
#include <SFML/Network/IpAddress.hpp>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

//Lobby* Lobby::lobby_instance = nullptr;

Lobby::Lobby()
{
    leave_button = CursorButton("LeaveGameButton.png");
    leave_button.GetSprite().setPosition(sf::Vector2f(472, 650));
    leave_button.RegisterLeftMouseDown(std::bind(&Lobby::leaveLobby, this));

    start_button = CursorButton("SplashStart.png");
    start_button.GetSprite().setPosition(sf::Vector2f(472, 550));
    start_button.RegisterLeftMouseDown(std::bind(&Lobby::startGame, this));

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

    if(ServerSocket.connect(sf::IpAddress("127.0.0.1"), network::SERVER_PORT, sf::seconds(1)) != sf::Socket::Status::Done)
    {
        cerr << "Server creation failed." << endl;
        return false;
    }

    ClientMessage::InitServer(ServerSocket, player_name);
//    Player::state.name = player_name;

    owner = true;

    initializeMenu();

    return true;
}

bool Lobby::Join(std::string player_name, std::string ip)
{
    if(ServerSocket.connect(sf::IpAddress(ip), network::SERVER_PORT, sf::seconds(1)) != sf::Socket::Status::Done)
    {
        cerr << "Server at " << ip << " could not be reached." << endl;
        return false;
    }

    ClientMessage::JoinServer(ServerSocket, player_name);
//    Player::state.name = player_name;

    owner = false;

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

    //for (auto& pair : player_display_list)
    //{
    //    GameManager::GetInstance().Window.draw(pair.second);
    //}
}
/*
void Lobby::AddPlayer(PlayerState player)
{
    sf::Text text;
    text.setFont(*font);
    text.setString(player.name);

    player_display_list[player.id] = text;

    updatePlayerPositions();
}

void Lobby::RemovePlayer(uint16_t player_id)
{
    player_display_list.erase(player_id);

    updatePlayerPositions();
}

void Lobby::ClearPlayers()
{
    player_display_list.clear();
}

void Lobby::updatePlayerPositions()
{
    int offset = 0;
    for (auto& pair : player_display_list)
    {
        pair.second.setPosition(sf::Vector2f(50, 50 + offset));
        offset += 35;
    }
}
*/
void Lobby::onMouseDown(sf::Event event)
{
    leave_button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
}

void Lobby::onMouseUp(sf::Event event)
{
    leave_button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
}

void Lobby::startGame()
{
    if (owner)
    {
        GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::LoadingScreen;
        //Message::StartGame();
        //StateManager::MainMenu::current_menu = MenuType::LoadingScreen;
        //GameManager::GetInstance().LoadGame();
    }
}

void Lobby::leaveLobby()
{
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseButtonPressed, mouse_down_id);
    EventHandler::GetInstance().UnregisterCallback(sf::Event::EventType::MouseButtonReleased, mouse_up_id);

    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::Main;
    //Message::LeaveGame();
    //StateManager::MainMenu::current_menu = MenuType::Main;
    //Players::Clear();
}

} // client
