#include "lobby.h"
#include "messaging.h"
#include <iostream>

using std::cout, std::cerr, std::endl;

Lobby::Lobby()
{
    leave_button = CursorButton("LeaveGameButton.png");
    leave_button.GetSprite().setPosition(sf::Vector2f(472, 650));
    leave_button.RegisterOnClickUp(std::bind(&Lobby::onLeavePressed, this));
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
    return true;
}

void Lobby::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    leave_button.Update(elapsed, window);
}

void Lobby::Draw(sf::RenderWindow& window)
{
    leave_button.Draw(window);
}

void Lobby::onLeavePressed()
{
    Message::LeaveGame();
    StateManager::MainMenu::current_menu = MenuType::Main;
}
