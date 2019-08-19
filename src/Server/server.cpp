#include "server.h"
#include "network.h"
#include <iostream>

using std::cout, std::cerr, std::endl;

Server::Server()
{
    listener.setBlocking(false);
    listener.listen(Network::ServerPort);
}

void Server::Update()
{
    listen();

    // Process any incoming messages
    for (auto& player : players)
    {
        checkMessages(player);
    }
}

void Server::listen()
{
    std::shared_ptr<sf::TcpSocket> temp_socket(new sf::TcpSocket);
    sf::Socket::Status status = listener.accept(*temp_socket);
    if (status == sf::Socket::Status::NotReady)
    {
        return;
    }
    else if (status != sf::Socket::Status::Done)
    {
        cerr << "Tcp Listener threw an error" << endl;
        return;
    }

    cout << "Accepting new player" << endl;

    PlayerData player;
    player.socket = temp_socket;
    player.socket->setBlocking(false);
    player.name = "";

    players.push_back(player);
}

void Server::checkMessages(PlayerData& player)
{
    Network::MessageType code;
    size_t bytes_received;

    player.socket->setBlocking(false);
    auto status = player.socket->receive(&code, 1, bytes_received);
    if (status == sf::Socket::Status::NotReady)
    {
        return;
    }
    else if (status != sf::Socket::Status::Done)
    {
        cerr << "checkMessages threw a socket error" << endl;
        return;
    }

    switch (code)
    {
        case Network::MessageType::Error:
        {
            cerr << "Error codes not yet implemented" << endl;
        }
        break;
        case Network::MessageType::SetName:
        {
            cout << "Change Name request received" << endl;
            setName(player);
        }
        break;
    }
}

void Server::setName(PlayerData& player)
{
    std::string name = Network::ReadString(*player.socket);
    player.name = name;
    if (name != "")
    {
        cout << "Player changed name to: " << name << endl;
        // TODO: Notify name change
    }
}
