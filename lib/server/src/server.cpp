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
#include "global_state.h"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
using server::global::PlayerList;

namespace server {

namespace {
    constexpr float MAX_BROADCAST_RATE = 120; // Hz
    constexpr float STARTING_BATTERY = 300;
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

                definitions::Projectile projectile;
                if (player.SpawnProjectile(projectile))
                {
                    region.Projectiles.push_front(projectile);
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
        case ClientMessage::Code::UseItem:
        {
            useItem(player);
        }
        break;
        case ClientMessage::Code::SwapItem:
        {
            swapItem(player);
        }
        break;
        case ClientMessage::Code::Console:
        {
            consoleInteract(player);
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

    zone = definitions::GetZone();
    current_region = definitions::STARTING_REGION;
    region = Region(zone.regions[current_region].type, PlayerList.size(), STARTING_BATTERY);

    for (unsigned i = 0; i < item_stash.size(); ++i)
    {
        if (i < 6)
        {
            item_stash[i] = definitions::ItemType::Medpack;
        }
        else
        {
            item_stash[i] = definitions::ItemType::None;
        }
    }

    for (auto& player : PlayerList)
    {
        ServerMessage::UpdateStash(*player.Socket, item_stash);
    }

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

        for (auto& p : PlayerList)
        {
            if (p.Status == Player::PlayerStatus::Loading)
            {
                return;
            }
        }

        double battery_cost = 0;
        for (auto& link : zone.links)
        {
            if ((link.start == current_region && link.finish == next_region) ||
                (link.finish == current_region && link.start == next_region))
            {
                battery_cost = link.distance;
                break;
            }
        }

        if (battery_cost == 0)
        {
            cerr << "Moved to a region not adjacent." << endl;
        }

        if (battery_cost > region.BatteryLevel)
        {
            cerr << "Moved to a region with insufficient battery power." << endl;
            battery_cost = region.BatteryLevel;
        }

        current_region = next_region;

        for (auto& node : zone.regions)
        {
            if (node.id == current_region)
            {
                region = Region(node.type, PlayerList.size(), region.BatteryLevel - battery_cost);
                break;
            }
        }

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

void Server::useItem(Player& player)
{
    player.UseItem();
}

void Server::swapItem(Player& player)
{
    uint8_t item_index;
    if (!ClientMessage::DecodeSwapItem(*player.Socket, item_index))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    definitions::ItemType item = player.ChangeItem(item_stash[item_index]);
    ServerMessage::ChangeItem(*player.Socket, item_stash[item_index]);
    item_stash[item_index] = item;

    for (auto& p : PlayerList)
    {
        ServerMessage::UpdateStash(*p.Socket, item_stash);
    }
}

void Server::consoleInteract(Player& player)
{
    bool activate;
    if (!ClientMessage::DecodeConsole(*player.Socket, activate))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    // TODO: Activating the console should not pause right away
    global::Paused = activate;

    for (auto& p : PlayerList)
    {
        ServerMessage::SetPaused(*p.Socket, global::Paused);
    }
}

void Server::changeRegion(Player& player)
{
    uint16_t region_id;
    if (!ClientMessage::DecodeChangeRegion(*player.Socket, region_id))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    next_region = region_id;

    for (auto& p : PlayerList)
    {
        p.Status = Player::PlayerStatus::Loading;
        ServerMessage::ChangeRegion(*p.Socket, region_id);
    }
}

void Server::broadcastStates()
{
    std::vector<network::PlayerData> player_list;
    std::vector<network::EnemyData> enemy_list;
    std::vector<network::ProjectileData> projectile_list;

    for (auto& player : PlayerList)
    {
        player_list.push_back(player.Data);
    }

    for (auto& enemy : region.Enemies)
    {
        enemy_list.push_back(enemy.Data);
    }

    for (auto& projectile : region.Projectiles)
    {
        network::ProjectileData data;
        data.id = projectile.id;
        data.position = projectile.position;
        projectile_list.push_back(data);
    }

    for (auto& player : PlayerList)
    {
        ServerMessage::PlayerStates(*player.Socket, player_list);
        ServerMessage::EnemyUpdate(*player.Socket, enemy_list);
        ServerMessage::BatteryUpdate(*player.Socket, region.BatteryLevel);
        ServerMessage::ProjectileUpdate(*player.Socket, projectile_list);
    }
}

} // server
