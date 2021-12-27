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
#include "util.h"
#include "player_list.h"

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

    if (owner != 0 && PlayerList.size() == 0)
    {
        cout << "Shutting down" << endl;
        running = false;
        return;
    }

    // Process any incoming messages
    for (auto iter = PlayerList.begin(); iter != PlayerList.end(); ++iter)
    {
        auto& player = *iter;
        if (player.Status != Player::PlayerStatus::Disconnected)
        {
            checkMessages(player);
        }
        else
        {
            PlayerList.erase(iter--);
        }
    }

    sf::Time elapsed = clock.restart();

    if (game_state == GameState::Game)
    {
        region.Update(elapsed);

        for (auto& player : PlayerList)
        {
            if (player.Status == Player::PlayerStatus::Alive)
            {
                player.Update(elapsed, region);
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

    Player player{};
    player.Socket = player_socket;
    player.Socket->setBlocking(false);
    player.Data.name = "";
    player.Data.properties.player_class = network::PlayerClass::Melee;
    player.Data.properties.weapon_type = definitions::WeaponType::Sword;
    player.SetWeapon(definitions::GetWeapon(player.Data.properties.weapon_type));
    player.Status = Player::PlayerStatus::Uninitialized;

    cout << "New player connected to server." << endl;

    PlayerList.push_back(player);
}

void Server::checkMessages(Player& player)
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
    float angle = 2 * util::pi / PlayerList.size();

    for (size_t i = 0; i < PlayerList.size(); ++i)
    {
        // TODO: Magic numbers for distance
        sf::Vector2f spawn_position(50 * std::sin(angle * i), 50 * std::cos(angle * i));
        ServerMessage::AllPlayersLoaded(*PlayerList[i].Socket, spawn_position);
        PlayerList[i].Data.position = spawn_position;
    }

    region = Region(definitions::STARTNG_REGION, PlayerList.size(), 0);

    game_state = GameState::Game;
}

void Server::initLobby(Player& player)
{
    if (!ClientMessage::DecodeInitLobby(*player.Socket, player.Data.name))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Uninitialized)
    {
        cerr << player.Data.name << " tried to initialize a server that was already initialized." << endl;
        return;
    }

    if (player.Status != Player::PlayerStatus::Uninitialized)
    {
        cerr << player.Data.name << " is already initialized." << endl;
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
        return;
    }

    uint16_t id = getPlayerUid();

    if (ServerMessage::PlayerId(*player.Socket, id))
    {
        player.Data.id = id;
        owner = player.Data.id;
        cout << "Server initialized by " << player.Data.name << "." << endl;
        player.Status = Player::PlayerStatus::Menus;
        game_state = GameState::Lobby;
    }
    else
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }
}

void Server::playerJoined(Player& player)
{
    if (!ClientMessage::DecodeJoinLobby(*player.Socket, player.Data.name))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Lobby)
    {
        cerr << player.Data.name << " tried to join a game that was not in the lobby." << endl;
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
        return;
    }

    if (player.Status != Player::PlayerStatus::Uninitialized)
    {
        cerr << player.Data.name << " is already initialized." << endl;
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
        return;
    }

    player.Data.id = getPlayerUid();
    std::vector<network::PlayerData> players_in_lobby;

    for (auto& p : PlayerList)
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
        player.Status = Player::PlayerStatus::Menus;
    }
    else
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }
}

void Server::changePlayerProperty(Player& player)
{
    if (!ClientMessage::DecodeChangePlayerProperty(*player.Socket, player.Data.properties))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
        return;
    }

    if (game_state != GameState::Lobby)
    {
        cerr << player.Data.name << " tried to change properties while not in the lobby." << endl;
        return;
    }

    player.SetWeapon(definitions::GetWeapon(player.Data.properties.weapon_type));

    for (auto& p : PlayerList)
    {
        if (p.Data.id != player.Data.id)
        {
            ServerMessage::ChangePlayerProperty(*p.Socket, player.Data.id, player.Data.properties);
        }
    }
}

void Server::startLoading(Player& player)
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

    for (auto& p : PlayerList)
    {
        if (p.Data.id != owner)
        {
            ServerMessage::StartGame(*p.Socket);
        }
    }
}

void Server::loadingComplete(Player& player)
{
    if (game_state == GameState::Loading)
    {
        cout << player.Data.name << " has finished loading." << endl;
        player.Status = Player::PlayerStatus::Alive;
        player.Data.health = 100;

        for (auto& p : PlayerList)
        {
            if (p.Status != Player::PlayerStatus::Alive)
            {
                return;
            }
        }

        startGame();

        cout << "All PlayerList have loaded!" << endl;
    }
    else if (game_state == GameState::Game)
    {
        cout << player.Data.name << " has finished loading the new region." << endl;
        player.Status = Player::PlayerStatus::Alive;
        player.Data.health = 100;

        for (auto& p : PlayerList)
        {
            if (p.Status == Player::PlayerStatus::Loading)
            {
                return;
            }
        }

        region = Region(next_region, PlayerList.size(), 0);

        // Determine spawn positions
        float angle = 2 * util::pi / PlayerList.size();

        for (size_t i = 0; i < PlayerList.size(); ++i)
        {
            // TODO: Magic numbers for distance
            sf::Vector2f spawn_position = sf::Vector2f(50 * std::sin(angle * i), 50 * std::cos(angle * i)) + region.Convoy.Position;
            spawn_position.x += 200;
            ServerMessage::AllPlayersLoaded(*PlayerList[i].Socket, spawn_position);
            PlayerList[i].Data.position = spawn_position;
        }
    }
}

void Server::leaveGame(Player& player)
{
    cout << player.Data.name << " left the server." << endl;
    player.Socket->disconnect();
    player.Status = Player::PlayerStatus::Disconnected;

    for (auto& p : PlayerList)
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

void Server::updatePlayerState(Player& player)
{
    sf::Vector2i movement_vector;
    if (!ClientMessage::DecodePlayerStateChange(*player.Socket, movement_vector))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Game)
    {
        cerr << "Server received a player update when not in game." << endl;
        return;
    }

    if (player.Status != Player::PlayerStatus::Alive)
    {
        cerr << "Player attempted to change states while not alive." << endl;
        return;
    }

    player.UpdatePlayerState(movement_vector);
}

void Server::startPlayerAction(Player& player)
{
    network::PlayerAction action;
    if (!ClientMessage::DecodeStartAction(*player.Socket, action))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    if (game_state != GameState::Game)
    {
        cerr << "Server received a player action when not in game." << endl;
        return;
    }

    if (player.Status != Player::PlayerStatus::Alive)
    {
        cerr << "Player attempted to take an action while not alive." << endl;
        return;
    }

    if (action.flags.start_attack)
    {
        player.StartAttack(action.attack_angle);
    }

    for (auto& p : PlayerList)
    {
        if (p.Data.id != player.Data.id)
        {
            ServerMessage::PlayerStartAction(*p.Socket, player.Data.id, action);
        }
    }
}

void Server::changeRegion(Player& player)
{
    definitions::RegionName region_name;
    if (!ClientMessage::DecodeChangeRegion(*player.Socket, region_name))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    next_region = region_name;

    for (auto& p : PlayerList)
    {
        p.Status = Player::PlayerStatus::Loading;
        ServerMessage::ChangeRegion(*p.Socket, region_name);
    }
}

void Server::broadcastStates()
{
    std::vector<network::PlayerData> player_list;
    std::vector<network::EnemyData> enemy_list;

    for (auto& player : PlayerList)
    {
        player_list.push_back(player.Data);
    }

    for (auto& enemy : region.Enemies)
    {
        enemy_list.push_back(enemy.Data);
    }

    for (auto& player : PlayerList)
    {
        ServerMessage::PlayerStates(*player.Socket, player_list);
        ServerMessage::EnemyUpdate(*player.Socket, enemy_list);
        ServerMessage::BatteryUpdate(*player.Socket, region.BatteryLevel);
    }
}

} // server
