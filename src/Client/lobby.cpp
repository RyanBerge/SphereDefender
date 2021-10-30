#include "lobby.h"
#include "messaging.h"
#include "state_manager.h"
#include "game_manager.h"
#include "player.h"
#include <iostream>

using std::cout, std::cerr, std::endl;

Lobby* Lobby::lobby_instance = nullptr;

Lobby::Lobby()
{
    lobby_instance = this;

    leave_button = CursorButton("LeaveGameButton.png");
    leave_button.GetSprite().setPosition(sf::Vector2f(472, 650));
    leave_button.RegisterOnClickUp(std::bind(&Lobby::onLeavePressed, this));

    start_button = CursorButton("SplashStart.png");
    start_button.GetSprite().setPosition(sf::Vector2f(472, 550));
    start_button.RegisterOnClickUp(std::bind(&Lobby::onStartPressed, this));

    font = Resources::AllocFont("assets/Vera.ttf");

    if (font == nullptr)
    {
        std::cerr << "Lobby: There was an error loading the font." << std::endl;
    }
}

bool Lobby::InitNew(std::string player_name)
{
    // launch server
    cerr << "Launching server..." << endl;
    std::system("start bin/SphereDefenderServer.exe");

    sf::Socket::Status status = Network::Connect("127.0.0.1");
    if (status != sf::Socket::Status::Done)
    {
        cerr << "Server creation failed." << endl;
        return false;
    }

    Message::InitializeServer(player_name);
    Player::state.name = player_name;
    owner = true;
    return true;
}

bool Lobby::InitJoin(std::string player_name, std::string ip)
{
    sf::Socket::Status status = Network::Connect(ip);
    if (status != sf::Socket::Status::Done)
    {
        cerr << "Server could not be reached." << endl;
        StateManager::MainMenu::current_menu = MenuType::Main;
        return false;
    }

    Message::JoinServer(player_name);
    owner = false;
    Player::state.name = player_name;
    return true;
}

void Lobby::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    leave_button.Update(elapsed, window);
    if (owner)
    {
        start_button.Update(elapsed, window);
    }
}

void Lobby::Draw(sf::RenderWindow& window)
{
    leave_button.Draw(window);
    if (owner)
    {
        start_button.Draw(window);
    }

    for (auto& pair : player_display_list)
    {
        window.draw(pair.second);
    }
}

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

void Lobby::onLeavePressed()
{
    Message::LeaveGame();
    StateManager::MainMenu::current_menu = MenuType::Main;
    Players::Clear();
}

void Lobby::onStartPressed()
{
    if (owner)
    {
        Message::StartGame();
        StateManager::MainMenu::current_menu = MenuType::LoadingScreen;
        GameManager::GetInstance().LoadGame();
    }
}
