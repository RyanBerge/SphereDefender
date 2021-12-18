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
            if (player.Status == PlayerInfo::PlayerStatus::Alive)
            {
                player.Update(elapsed, region.Obstacles, region.Convoy);
                if (player.Attacking)
                {
                    checkAttack(player);
                }
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

    PlayerInfo player{};
    player.Socket = player_socket;
    player.Socket->setBlocking(false);
    player.Data.name = "";
    player.Data.properties.player_class = network::PlayerClass::Melee;
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
        case ClientMessage::Code::ChangePlayerProperty:
        {
            changePlayerProperty(player);
        }
        break;
        case ClientMessage::Code::StartGame:
        {
            startLoading(player);
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
        case ClientMessage::Code::ChangeRegion:
        {
            changeRegion(player);
        }
        break;
        default:
        {
            cerr << "Unrecognized code." << endl;
        }
        break;
    }
}

void Server::startGame()
{
    // Determine spawn positions
    float angle = 2 * util::pi / players.size();

    for (size_t i = 0; i < players.size(); ++i)
    {
        // TODO: Magic numbers for distance
        sf::Vector2f spawn_position(50 * std::sin(angle * i), 50 * std::cos(angle * i));
        ServerMessage::AllPlayersLoaded(*players[i].Socket, spawn_position);
        players[i].Data.position = spawn_position;
    }

    region = Region(shared::RegionName::Town, players.size(), 0);

    game_state = GameState::Game;
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

void Server::changePlayerProperty(PlayerInfo& player)
{
    if (game_state != GameState::Lobby)
    {
        cerr << player.Data.name << " tried to change properties while not in the lobby." << endl;
        return;
    }

    if (!ClientMessage::DecodeChangePlayerProperty(*player.Socket, player.Data.properties))
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
        return;
    }

    for (auto& p : players)
    {
        if (p.Data.id != player.Data.id)
        {
            ServerMessage::ChangePlayerProperty(*p.Socket, player.Data.id, player.Data.properties);
        }
    }
}

void Server::startLoading(PlayerInfo& player)
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
        player.Data.health = 100;

        for (auto& p : players)
        {
            if (p.Status != PlayerInfo::PlayerStatus::Alive)
            {
                return;
            }
        }

        startGame();

        cout << "All players have loaded!" << endl;
    }
    else if (game_state == GameState::Game)
    {
        cout << player.Data.name << " has finished loading the new region." << endl;
        player.Status = PlayerInfo::PlayerStatus::Alive;
        player.Data.health = 100;

        for (auto& p : players)
        {
            if (p.Status == PlayerInfo::PlayerStatus::Loading)
            {
                return;
            }
        }

        region = Region(next_region, players.size(), 0);

        // Determine spawn positions
        float angle = 2 * util::pi / players.size();

        for (size_t i = 0; i < players.size(); ++i)
        {
            // TODO: Magic numbers for distance
            sf::Vector2f spawn_position = sf::Vector2f(50 * std::sin(angle * i), 50 * std::cos(angle * i)) + region.Convoy.position;
            spawn_position.x += 200;
            ServerMessage::AllPlayersLoaded(*players[i].Socket, spawn_position);
            players[i].Data.position = spawn_position;
        }
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

    if (player.Status != PlayerInfo::PlayerStatus::Alive)
    {
        cerr << "Player attempted to change states while not alive." << endl;
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

    if (player.Status != PlayerInfo::PlayerStatus::Alive)
    {
        cerr << "Player attempted to take an action while not alive." << endl;
        return;
    }

    if (action.flags.start_attack)
    {
        player.StartAttack(action.attack_angle);
    }

    for (auto& p : players)
    {
        if (p.Data.id != player.Data.id)
        {
            ServerMessage::PlayerStartAction(*p.Socket, player.Data.id, action);
        }
    }
}

void Server::changeRegion(PlayerInfo& player)
{
    shared::RegionName region_name;
    if (!ClientMessage::DecodeChangeRegion(*player.Socket, region_name))
    {
        player.Socket->disconnect();
        player.Status = PlayerInfo::PlayerStatus::Disconnected;
    }

    next_region = region_name;

    for (auto& p : players)
    {
        p.Status = PlayerInfo::PlayerStatus::Loading;
        ServerMessage::ChangeRegion(*p.Socket, region_name);
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

    for (auto& enemy : region.Enemies)
    {
        enemy_list.push_back(enemy.Data);
    }

    for (auto& player : players)
    {
        ServerMessage::PlayerStates(*player.Socket, player_list);
        ServerMessage::EnemyUpdate(*player.Socket, enemy_list);
        ServerMessage::BatteryUpdate(*player.Socket, region.BatteryLevel);
    }
}

void Server::checkAttack(PlayerInfo& player)
{
    switch (player.Data.properties.player_class)
    {
        case network::PlayerClass::Melee:
        {
            util::LineSegment sword = player.GetSwordLocation();

            for (auto& enemy : region.Enemies)
            {
                sf::FloatRect bounds = enemy.GetBounds();

                if (util::Contains(bounds, sword.p1) || util::Contains(bounds, sword.p2))
                {
                    enemy.WeaponHit(player.Data.id, player.GetWeaponDamage(), player.GetWeaponKnockback(), enemy.Data.position - player.Data.position, players);
                }
            }
        }
        break;
        case network::PlayerClass::Ranged:
        {
            sf::Vector2f attack_vector = player.GetAttackVector();

            bool collision = false;
            sf::Vector2f point;
            for (auto& rect : region.Obstacles)
            {
                sf::Vector2f temp;
                if (util::IntersectionPoint(rect, util::LineVector{player.Data.position, attack_vector}, temp))
                {
                    if (!collision)
                    {
                        collision = true;
                        point = temp;
                    }
                    else if (util::Distance(player.Data.position, temp) < util::Distance(player.Data.position, point))
                    {
                        point = temp;
                    }
                }
            }

            uint16_t target_enemy_id = region.Enemies.front().Data.id;
            bool enemy_hit = false;
            for (auto& enemy : region.Enemies)
            {
                sf::Vector2f temp;
                if (util::IntersectionPoint(enemy.GetBounds(), util::LineVector{player.Data.position, attack_vector}, temp))
                {
                    if (!collision)
                    {
                        collision = true;
                        point = temp;
                        enemy_hit = true;
                        target_enemy_id = enemy.Data.id;
                    }
                    else if (util::Distance(player.Data.position, temp) < util::Distance(player.Data.position, point))
                    {
                        point = temp;
                        enemy_hit = true;
                        target_enemy_id = enemy.Data.id;
                    }
                }
            }

            if (enemy_hit)
            {
                for (auto& enemy : region.Enemies)
                {
                    if (enemy.Data.id == target_enemy_id)
                    {
                        if (enemy.Data.health <= 10)
                        {
                            enemy.Data.health = 0;
                        }
                        else
                        {
                            enemy.WeaponHit(player.Data.id, player.GetWeaponDamage(), player.GetWeaponKnockback(), enemy.Data.position - player.Data.position, players);
                        }
                    }
                }
            }

            player.Attacking = false;
        }
        break;
    }
}

} // server
