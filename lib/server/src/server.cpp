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
#include <cmath>

#include "messaging.h"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

namespace server {

namespace {
    constexpr double pi = 3.141592653589793238462643383279502884L;
} // anonymous namespace

Server::Server()
{
    listener.setBlocking(false);
    listener.listen(49879); // TODO: Make server settings and load from a file
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

    if (game_state == GameState::Game)
    {
        if (clock.getElapsedTime().asSeconds() > 1.0 / 50)
        {
            broadcastStates();
            clock.restart();
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
    player.data.name = "";
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
        cerr << "Player disconnected unexpectedly: " << player.data.name << endl;
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
        case ClientMessage::Code::PlayerState:
        {
            updatePlayerState(player);
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
    if (!ClientMessage::DecodeInitLobby(*player.socket, player.data.name))
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }

    if (game_state != GameState::Uninitialized)
    {
        cerr << player.data.name << " tried to initialize a server that was already initialized." << endl;
        return;
    }

    if (player.status != PlayerInfo::Status::Uninitialized)
    {
        cerr << player.data.name << " is already initialized." << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
        return;
    }

    uint16_t id = getPlayerUid();

    if (ServerMessage::PlayerId(*player.socket, id))
    {
        player.data.id = id;
        owner = player.data.id;
        cout << "Server initialized by " << player.data.name << "." << endl;
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
    if (!ClientMessage::DecodeJoinLobby(*player.socket, player.data.name))
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }

    if (game_state != GameState::Lobby)
    {
        cerr << player.data.name << " tried to join a game that was not in the lobby." << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
        return;
    }

    if (player.status != PlayerInfo::Status::Uninitialized)
    {
        cerr << player.data.name << " is already initialized." << endl;
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
        return;
    }

    player.data.id = getPlayerUid();
    std::vector<network::PlayerData> players_in_lobby;

    for (auto& p : players)
    {
        if (player.data.id != p.data.id)
        {
            if (p.data.id == owner)
            {
                players_in_lobby.insert(players_in_lobby.begin(), p.data);
            }
            else
            {
                players_in_lobby.push_back(p.data);
            }

            ServerMessage::PlayerJoined(*p.socket, player.data);
        }
    }

    if (ServerMessage::PlayersInLobby(*player.socket, player.data.id, players_in_lobby))
    {
        cout << player.data.name << " joined the lobby" << endl;
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
        cerr << player.data.name << " tried to start a game that wasn't in the lobby." << endl;
        return;
    }

    if (player.data.id != owner)
    {
        cerr << player.data.name << " tried to start a game without being the owner." << endl;
        return;
    }

    cout << player.data.name << " started the game!" << endl;
    game_state = GameState::Loading;

    for (auto& p : players)
    {
        if (p.data.id != owner)
        {
            ServerMessage::StartGame(*p.socket);
        }
    }
}

void Server::loadingComplete(PlayerInfo& player)
{
    if (game_state == GameState::Loading)
    {
        cout << player.data.name << " has finished loading." << endl;
        player.status = PlayerInfo::Status::Alive;

        for (auto& p : players)
        {
            if (p.status != PlayerInfo::Status::Alive)
            {
                return;
            }
        }

        // Determine spawn positions
        float angle = 2 * pi / players.size();

        for (size_t i = 0; i < players.size(); ++i)
        {
            // TODO: Magic numbers for distance
            sf::Vector2f spawn_position(50 * std::sin(angle * i), 50 * std::cos(angle * i));
            ServerMessage::AllPlayersLoaded(*players[i].socket, spawn_position);
        }

        game_state = GameState::Game;

        cout << "All players have loaded!" << endl;
    }
}

void Server::leaveGame(PlayerInfo& player)
{
    cout << player.data.name << " left the server." << endl;
    player.socket->disconnect();
    player.status = PlayerInfo::Status::Disconnected;

    for (auto& p : players)
    {
        if (p.data.id != player.data.id)
        {
            if (player.data.id == owner)
            {
                ServerMessage::OwnerLeft(*p.socket);
            }
            else
            {
                ServerMessage::PlayerLeft(*p.socket, player.data.id);
            }
        }
    }
}

void Server::updatePlayerState(PlayerInfo& player)
{
    sf::Vector2f position;
    if (!ClientMessage::DecodePlayerState(*player.socket, position))
    {
        player.socket->disconnect();
        player.status = PlayerInfo::Status::Disconnected;
    }

    if (game_state != GameState::Game)
    {
        cerr << "wtf" << endl;
        return;
    }

    player.data.position = position;
}

void Server::broadcastStates()
{
    std::vector<network::PlayerData> player_list;

    for (auto& player : players)
    {
        player_list.push_back(player.data);
    }

    for (auto& player : players)
    {
        ServerMessage::PlayerStates(*player.socket, player_list);
    }
}

} // server
