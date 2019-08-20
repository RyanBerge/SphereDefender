#include "lobby.h"
#include "messaging.h"
#include <iostream>

using std::cout, std::cerr, std::endl;

Lobby::Lobby()
{
}

void Lobby::InitNew(std::string player_name)
{
    // launch server
    cerr << "Launching server..." << endl;
    std::system("start bin/Server.exe");

    sf::Socket::Status status = Network::Connect("127.0.0.1");
    if (status != sf::Socket::Status::Done)
    {
        cerr << "Server could not be reached." << endl;
        return;
    }

    Message::setName(player_name);
}

void Lobby::InitExisting(std::string player_name, std::string ip)
{
    sf::Socket::Status status = Network::Connect(ip);
    if (status != sf::Socket::Status::Done)
    {
        cerr << "Server could not be reached." << endl;
        return;
    }

    Message::setName(player_name);
}

void Lobby::Update(sf::Time elapsed, sf::RenderWindow& window)
{

}

void Lobby::Draw(sf::RenderWindow& window)
{

}
