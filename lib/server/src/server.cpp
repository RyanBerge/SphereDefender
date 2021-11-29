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
#include "game_math.h"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

namespace server {

namespace {
    constexpr float MAX_BROADCAST_RATE = 120; // Hz
} // anonymous namespace

Server::Server()
{
    listener.setBlocking(false);
    listener.listen(49879); // TODO: Make server settings and load from a file
}

void Server::Start()
{
    running = true;

    clock.restart();
    while (running)
    {
        update();
    }

    //while(true);
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
        if (player.Status != PlayerInfo::PlayerStatus::Disconnected)
        {
            checkMessages(player);
        }
        else
        {
            players.erase(iter--);
        }
    }

    sf::Time elapsed = clock.restart();

    if (game_state == GameState::Game)
    {
        region.Update(elapsed, players);

        for (auto& player : players)
        {
            player.Update(elapsed);
            if (player.Attacking)
            {
                checkAttack(player);
            }
        }

        static float broadcast_delta = 0;
        broadcast_delta += elapsed.asSeconds();

        if (broadcast_delta > 1 / MAX_BROADCAST_RATE)
        {
            broadcastStates();
            broadcast_delta = 0;
            region.Cull();
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
    player.Socket = player_socket;
    player.Socket->setBlocking(false);
    player.Data.name = "";
    player.Status = PlayerInfo::PlayerStatus::Uninitialized;

    cout << "New player connected to server." << endl;

    players.push_back(player);
}

void Server::checkMessages(PlayerInfo& player)
{
    ClientMessage::Code code;
    bool success = ClientMessage::PollForCode(*player.Socket, code);
    if (!success)
    {
        cerr << "Player disconnected unexpectedly: " << player.Data.name << endl;
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
        case ClientMessage::Code::PlayerStateChange:
        {
            updatePlayerState(player);
        }
        break;
        case ClientMessage::Code::StartAction:
        {
            startPlayerAction(player);
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
    if (!ClientMessage::DecodeInitLobby(*player.Socket, player.Data.name))
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Uninitialized)
    {
        cerr << player.Data.name << " tried to initialize a server that was already initialized." << endl;
        return;
    }

    if (player.Status != PlayerInfo::PlayerStatus::Uninitialized)
    {
        cerr << player.Data.name << " is already initialized." << endl;
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
        return;
    }

    uint16_t id = getPlayerUid();

    if (ServerMessage::PlayerId(*player.Socket, id))
    {
        player.Data.id = id;
        owner = player.Data.id;
        cout << "Server initialized by " << player.Data.name << "." << endl;
        player.Status = PlayerInfo::PlayerStatus::Menus;
        game_state = GameState::Lobby;
    }
    else
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }
}

void Server::playerJoined(PlayerInfo& player)
{
    if (!ClientMessage::DecodeJoinLobby(*player.Socket, player.Data.name))
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Lobby)
    {
        cerr << player.Data.name << " tried to join a game that was not in the lobby." << endl;
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
        return;
    }

    if (player.Status != PlayerInfo::PlayerStatus::Uninitialized)
    {
        cerr << player.Data.name << " is already initialized." << endl;
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
        return;
    }

    player.Data.id = getPlayerUid();
    std::vector<network::PlayerData> players_in_lobby;

    for (auto& p : players)
    {
        if (player.Data.id != p.Data.id)
        {
            if (p.Data.id == owner)
            {
                players_in_lobby.insert(players_in_lobby.begin(), p.Data);
            }
            else
            {
                players_in_lobby.push_back(p.Data);
            }

            ServerMessage::PlayerJoined(*p.Socket, player.Data);
        }
    }

    if (ServerMessage::PlayersInLobby(*player.Socket, player.Data.id, players_in_lobby))
    {
        cout << player.Data.name << " joined the lobby" << endl;
        player.Status = PlayerInfo::PlayerStatus::Menus;
    }
    else
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }
}

void Server::startGame(PlayerInfo& player)
{
    if (game_state != GameState::Lobby)
    {
        cerr << player.Data.name << " tried to start a game that wasn't in the lobby." << endl;
        return;
    }

    if (player.Data.id != owner)
    {
        cerr << player.Data.name << " tried to start a game without being the owner." << endl;
        return;
    }

    cout << player.Data.name << " started the game!" << endl;
    game_state = GameState::Loading;

    for (auto& p : players)
    {
        if (p.Data.id != owner)
        {
            ServerMessage::StartGame(*p.Socket);
        }
    }
}

void Server::loadingComplete(PlayerInfo& player)
{
    if (game_state == GameState::Loading)
    {
        cout << player.Data.name << " has finished loading." << endl;
        player.Status = PlayerInfo::PlayerStatus::Alive;

        for (auto& p : players)
        {
            if (p.Status != PlayerInfo::PlayerStatus::Alive)
            {
                return;
            }
        }

        // Determine spawn positions
        float angle = 2 * util::pi / players.size();

        for (size_t i = 0; i < players.size(); ++i)
        {
            // TODO: Magic numbers for distance
            sf::Vector2f spawn_position(50 * std::sin(angle * i), 50 * std::cos(angle * i));
            ServerMessage::AllPlayersLoaded(*players[i].Socket, spawn_position);
            players[i].Data.position = spawn_position;
        }

        game_state = GameState::Game;
        initializeRegion();

        cout << "All players have loaded!" << endl;
    }
}

void Server::leaveGame(PlayerInfo& player)
{
    cout << player.Data.name << " left the server." << endl;
    player.Socket->disconnect();
    player.Status = PlayerInfo::PlayerStatus::Disconnected;

    for (auto& p : players)
    {
        if (p.Data.id != player.Data.id)
        {
            if (player.Data.id == owner)
            {
                ServerMessage::OwnerLeft(*p.Socket);
            }
            else
            {
                ServerMessage::PlayerLeft(*p.Socket, player.Data.id);
            }
        }
    }
}

void Server::updatePlayerState(PlayerInfo& player)
{
    sf::Vector2i movement_vector;
    if (!ClientMessage::DecodePlayerStateChange(*player.Socket, movement_vector))
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Game)
    {
        cerr << "Server received a player update when not in game." << endl;
        return;
    }

    player.UpdatePlayerState(movement_vector);
}

void Server::startPlayerAction(PlayerInfo& player)
{
    network::PlayerAction action;
    if (!ClientMessage::DecodeStartAction(*player.Socket, action))
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Game)
    {
        cerr << "Server received a player action when not in game." << endl;
        return;
    }

    if (action.start_attack)
    {
        player.StartAttack(action.attack_angle);
    }

    for (auto& p : players)
    {
        if (p.Data.id != player.Data.id)
        {
            ServerMessage::StartAction(*p.Socket, player.Data.id, action);
        }
    }
}

void Server::broadcastStates()
{
    std::vector<network::PlayerData> player_list;
    std::vector<network::EnemyData> enemy_list;

    for (auto& player : players)
    {
        player_list.push_back(player.Data);
    }

    for (auto& enemy : region.enemies)
    {
        enemy_list.push_back(enemy.Data);
    }

    for (auto& player : players)
    {
        ServerMessage::PlayerStates(*player.Socket, player_list);
        ServerMessage::EnemyUpdate(*player.Socket, enemy_list);
    }
}

void Server::initializeRegion()
{
    region = Region();

    std::vector<network::EnemyData> enemies;
    for (auto& enemy : region.enemies)
    {
        enemies.push_back(enemy.Data);
    }

    for (auto& player : players)
    {
        ServerMessage::RegionInfo(*player.Socket, enemies);
    }
}

void Server::checkAttack(PlayerInfo& player)
{
    util::LineSegment sword = player.GetSwordLocation();

    for (auto& enemy : region.enemies)
    {
        sf::FloatRect bounds;
        bounds.left = enemy.Data.position.x;
        bounds.top = enemy.Data.position.y;
        bounds.width = enemy.Bounds.x;
        bounds.height = enemy.Bounds.y;

        if (util::Contains(bounds, sword.p1) || util::Contains(bounds, sword.p2))
        {
            enemy.Data.health = 0;
        }
    }
}

} // server
