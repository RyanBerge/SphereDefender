/**************************************************************************************************
 *  File:       server.cpp
 *  Class:      Server
 *
 *  Purpose:    The main server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "server.h"
#include <cstring>
#include <iostream>

#include "messaging.h"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

namespace server {

Server::Server()
{
    listener.setBlocking(false);
    listener.listen(network::SERVER_PORT);
}

void Server::Start()
{
    running = true;

    while (running)
    {
        update();
    }
}

uint16_t Server::getPlayerUid()
{
    // 0 is reserved
    static uint16_t player_id = 1;
    return player_id++;
}

void Server::update()
{
    listen();

    if (owner != 0 && players.size() == 0)
    {
        cout << "Shutting down" << endl;
        running = false;
        return;
    }

    // Process any incoming messages
    for (auto iter = players.begin(); iter != players.end(); ++iter)
    {
        auto& player = *iter;
        if (player.status != PlayerInfo::Status::Disconnected)
        {
            checkMessages(player);
        }
        else
        {
            players.erase(iter--);
        }
    }
}

void Server::listen()
{
    std::shared_ptr<sf::TcpSocket> player_socket(new sf::TcpSocket);
    sf::Socket::Status status = listener.accept(*player_socket);
    if (status == sf::Socket::Status::NotReady)
    {
        return;
    }
    else if (status != sf::Socket::Status::Done)
    {
        cerr << "Tcp Listener threw an error" << endl;
        return;
    }

    PlayerInfo player;
    player.socket = player_socket;
    player.socket->setBlocking(false);
    player.name = "";
    player.status = PlayerInfo::Status::Uninitialized;

    cout << "New player connected to server." << endl;

    players.push_back(player);
}

void Server::checkMessages(PlayerInfo& player)
{
    ClientMessage::Code code;
    bool success = ClientMessage::PollForCode(*player.socket, code);
    if (!success)
    {
        cerr << "Player disconnected unexpectedly: " << player.name << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;

        leaveGame(player);
        return;
    }

    if (code == ClientMessage::Code::None)
    {
        return;
    }

    switch (code)
    {
        case ClientMessage::Code::Error:
        {
            cerr << "Error codes not yet implemented." << endl;
        }
        break;
        case ClientMessage::Code::InitLobby:
        {
            initLobby(player);
        }
        break;
        case ClientMessage::Code::JoinLobby:
        {
            playerJoined(player);
        }
        break;
        case ClientMessage::Code::StartGame:
        {
            startGame(player);
        }
        break;
        case ClientMessage::Code::LoadingComplete:
        {
            loadingComplete(player);
        }
        break;
        case ClientMessage::Code::LeaveGame:
        {
            leaveGame(player);
        }
        break;
        default:
        {
            cerr << "Unrecognized code." << endl;
        }
        break;
    }
}

void Server::initLobby(PlayerInfo& player)
{
    if (!ClientMessage::DecodeInitLobby(*player.socket, player.name))
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }

    if (game_state != GameState::Uninitialized)
    {
        cerr << player.name << " tried to initialize a server that was already initialized." << endl;
        return;
    }

    if (player.status != PlayerInfo::Status::Uninitialized)
    {
        cerr << player.name << " is already initialized." << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
        return;
    }

    uint16_t id = getPlayerUid();

    if (ServerMessage::PlayerId(*player.socket, id))
    {
        player.id = id;
        owner = player.id;
        cout << "Server initialized by " << player.name << "." << endl;
        player.status = PlayerInfo::Status::Menus;
        game_state = GameState::Lobby;
    }
    else
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }
}

void Server::playerJoined(PlayerInfo& player)
{
    if (!ClientMessage::DecodeJoinLobby(*player.socket, player.name))
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }

    if (game_state != GameState::Lobby)
    {
        cerr << player.name << " tried to join a game that was not in the lobby." << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
        return;
    }

    if (player.status != PlayerInfo::Status::Uninitialized)
    {
        cerr << player.name << " is already initialized." << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
        return;
    }

    player.id = getPlayerUid();
    std::vector<network::PlayerData> players_in_lobby;

    for (auto& p : players)
    {
        if (player.id != p.id)
        {
            if (p.id == owner)
            {
                players_in_lobby.insert(players_in_lobby.begin(), network::PlayerData{p.id, p.name});
            }
            else
            {
                players_in_lobby.push_back(network::PlayerData{p.id, p.name});
            }

            ServerMessage::PlayerJoined(*p.socket, player.name, player.id);
        }
    }

    if (ServerMessage::PlayersInLobby(*player.socket, player.id, players_in_lobby))
    {
        cout << player.name << " joined the lobby" << endl;
        player.status = PlayerInfo::Status::Menus;
    }
    else
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }
}

void Server::startGame(PlayerInfo& player)
{
    if (game_state != GameState::Lobby)
    {
        cerr << player.name << " tried to start a game that wasn't in the lobby." << endl;
        return;
    }

    if (player.id != owner)
    {
        cerr << player.name << " tried to start a game without being the owner." << endl;
        return;
    }

    cout << player.name << " started the game!" << endl;
    game_state = GameState::Loading;

    for (auto& p : players)
    {
        if (p.id != owner)
        {
            ServerMessage::StartGame(*p.socket);
        }
    }
}

void Server::loadingComplete(PlayerInfo& player)
{
    if (game_state == GameState::Loading)
    {
        cout << player.name << " has finished loading." << endl;
        player.status = PlayerInfo::Status::Alive;

        for (auto& p : players)
        {
            if (p.status != PlayerInfo::Status::Alive)
            {
                return;
            }
        }

        cout << "All players have loaded!" << endl;

        for (auto& p : players)
        {
            ServerMessage::AllPlayersLoaded(*p.socket);
        }

        game_state = GameState::Game;
    }
}

void Server::leaveGame(PlayerInfo& player)
{
    cout << player.name << " left the server." << endl;
    player.socket->disconnect();
    player.status = PlayerInfo::Status::Disconnected;

    for (auto& p : players)
    {
        if (p.id != player.id)
        {
            if (player.id == owner)
            {
                ServerMessage::OwnerLeft(*p.socket);
            }
            else
            {
                ServerMessage::PlayerLeft(*p.socket, player.id);
            }
        }
    }
}

} // server
