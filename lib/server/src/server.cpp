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
#include <set>
#include <ctime>
#include "messaging.h"
#include "game_math.h"
#include "util.h"
#include "global_state.h"
#include "debug_overrides.h"

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
    sf::Socket::Status status = listener.listen(49179); // TODO: Make server settings and load from a file
    if (status != sf::Socket::Status::Done)
    {
        std::cerr << "Tcp Listener failed to initialize." << std::endl;
    }
}

void Server::Start()
{
    running = true;

    clock.restart();

    try
    {
        while (running)
        {
            update();
        }
    }
    catch (const std::exception& e)
    {
        cerr << "Server threw an exception: " << e.what() << endl;
        while(true); // loop so you can read the console
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

    if (PlayerList.size() == 0)
    {
        return;
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

        if (global::RegionSelect)
        {
            checkVotes(VotingType::RegionSelect);
        }
        else if (global::MenuEvent)
        {
            checkVotes(VotingType::MenuEvent);
        }

        if (global::GatheringPlayers)
        {
            gatherPlayers();
        }

        static float broadcast_delta = 0;
        broadcast_delta += elapsed.asSeconds();

        if (broadcast_delta > 1 / MAX_BROADCAST_RATE)
        {
            broadcastStates();
            broadcast_delta = 0;
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
        cerr << "Tcp Listener threw an error: " << status << endl;
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
        case ClientMessage::Code::CastVote:
        {
            castVote(player);
        }
        break;
        case ClientMessage::Code::Console:
        {
            consoleInteract(player);
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
        sf::Vector2f spawn_point{0, 0};
#ifndef NDEBUG
        if (debug::PlayerSpawnPoint.override)
        {
            spawn_point = debug::PlayerSpawnPoint.value;
        }
#endif
        // TODO: Magic numbers for distance
        sf::Vector2f spawn_position(50 * std::sin(angle * i), 50 * std::cos(angle * i));
        spawn_position += spawn_point;
        ServerMessage::AllPlayersLoaded(*PlayerList[i].Socket, spawn_position);
        PlayerList[i].Data.position = spawn_position;
    }

    current_region = debug::StartingRegion.value;

    region.~Region();
    new(&region)Region(current_zone.regions[current_region].type, PlayerList.size(), STARTING_BATTERY);

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

definitions::Zone Server::generateZone()
{
    // Zone boundaries are an abstract 1200x600 units that are then scaled to fit in the GUI overmap
    // Abstract zone coordinates start bottom-left at (0, 0) and progress up and right

#ifndef NDEBUG
    if (debug::StaticMap.override && debug::StaticMap.value)
    {
        return createTestZone();
    }
#endif

    definitions::Zone new_zone;

    sf::Vector2f node_count{definitions::Zone::ZONE_WIDTH / 200, definitions::Zone::ZONE_HEIGHT / 200};

    int num_deleted_nodes = util::GetRandomInt(std::ceil(node_count.x * node_count.y * 0.25f), std::floor(node_count.x * node_count.y * 0.4f));
    std::vector<int> full_node_set;
    for (int i = 1; i < node_count.x * node_count.y - 1; ++i)
    {
        full_node_set.push_back(i);
    }
    std::set<int> deleted_node_set;

    srand(std::time(nullptr));
    std::random_shuffle(full_node_set.begin(), full_node_set.end());
    for (int i = 0; i < num_deleted_nodes; ++i)
    {
        deleted_node_set.insert(full_node_set[i]);
    }

    uint16_t node_id = 0;
    for (float x = 0; x < node_count.x; ++x)
    {
        for (float y = 0; y < node_count.y; y++)
        {
            bool first_node = (x == 0 && y == 0);
            bool last_node = (x == node_count.x - 1 && y == node_count.y - 1);
            definitions::Zone::RegionNode node{};
            node.id = node_id++;
            if (!first_node && !last_node && deleted_node_set.contains(node.id))
            {
                deleted_node_set.extract(node.id);
                continue;
            }

            float roll = util::GetRandomFloat(0, 100);

            if (roll < 10)
            {
                node.type = definitions::RegionType::Town;
            }
            else if (roll < 40)
            {
                node.type = definitions::RegionType::MenuEvent;
            }
            else
            {
                node.type = definitions::RegionType::Neutral;
            }

            if (x == 0 && y == 0)
            {
                node.type = definitions::RegionType::StartingTown;
            }

            sf::Vector2f offsets{util::GetRandomFloat(20, 180), util::GetRandomFloat(40, 160)};
            node.coordinates = sf::Vector2f{x * 200 + offsets.x, y * 200 + offsets.y};

            new_zone.regions.push_back(node);
        }
    }

    int num_leylines = util::GetRandomInt(std::floor((node_count.x * node_count.y) / 10), std::ceil((node_count.x * node_count.y) / 10));
    unsigned interval = new_zone.regions.size() / (num_leylines + 1.0f);

    for (int i = 0; i < num_leylines; ++i)
    {
        int offset = util::GetRandomInt(-1, 1);
        unsigned leyline_index = interval * (i + 1) + offset;

        while (leyline_index < new_zone.regions.size() && new_zone.regions[leyline_index].type == definitions::RegionType::Leyline)
        {
            ++leyline_index;
        }

        if (leyline_index >= new_zone.regions.size())
        {
            continue;
        }

        new_zone.regions[leyline_index].type = definitions::RegionType::Leyline;
    }

    generateLinksSimpleDiagonal(new_zone);

    return new_zone;
}

#ifndef NDEBUG
definitions::Zone Server::createTestZone()
{
    definitions::Zone new_zone;
    sf::Vector2f node_count{definitions::Zone::ZONE_WIDTH / 200, definitions::Zone::ZONE_HEIGHT / 200};

    uint16_t node_id = 0;
    for (float x = 0; x < node_count.x; ++x)
    {
        for (float y = 0; y < node_count.y; y++)
        {
            definitions::Zone::RegionNode node{};

            node.id = node_id++;
            node.coordinates = sf::Vector2f{x * 200 + 100, y * 200 + 100};

            switch (node.id)
            {
                case 0:
                {
                    node.type = definitions::RegionType::StartingTown;
                }
                break;
                case 10:
                {
                    node.type = definitions::RegionType::MenuEvent;
                }
                break;
                case 1: [[fallthrough]];
                case 11:
                {
                    node.type = definitions::RegionType::Leyline;
                }
                break;
                case 14:
                {
                    node.type = definitions::RegionType::Town;
                }
                break;
                case 29:
                {
                    node.type = definitions::RegionType::Secret;
                }
                break;
                default:
                {
                    node.type = definitions::RegionType::Neutral;
                }
                break;
            }

            new_zone.regions.push_back(node);
        }
    }

    generateLinksSimpleDiagonal(new_zone);

    return new_zone;
}
#endif

std::optional<definitions::Zone::RegionNode> Server::getNodeById(definitions::Zone& new_zone, uint16_t node_id)
{
    //for (unsigned i = std::min(node_id, static_cast<uint16_t>(new_zone.regions.size() - 1)); new_zone.regions[i].id >= i; --i)
    for (unsigned i = 0; i < new_zone.regions.size(); ++i)
    {
        if (new_zone.regions[i].id == node_id)
        {
            return new_zone.regions[i];
        }
    }

    return std::nullopt;
}

void Server::generateLinksSimpleDiagonal(definitions::Zone& zone)
{
    sf::Vector2f node_count{definitions::Zone::ZONE_WIDTH / 200, definitions::Zone::ZONE_HEIGHT / 200};

    for (int x = 0; x < node_count.x; ++x)
    {
        for (int y = 0; y < node_count.y; ++y)
        {
            uint16_t node_id = y + x * node_count.y;
            if (!getNodeById(zone, node_id).has_value())
            {
                continue;
            }

            for (int target_x = x + 1; target_x < node_count.x; ++target_x)
            {
                int target_y = y;
                uint16_t target_id = target_y + target_x * node_count.y;

                if (addLink(zone, node_id, target_id))
                {
                    break;
                }

                if (target_y - 1 >= 0)
                {
                    target_id = (target_y - 1) + target_x * node_count.y;
                    addLink(zone, node_id, target_id);
                }

                if (target_y + 1 < node_count.y)
                {
                    target_id = (target_y + 1) + target_x * node_count.y;
                    addLink(zone, node_id, target_id);
                }
            }

            for (int target_y = y + 1; target_y < node_count.y; ++target_y)
            {
                int target_x = x;
                uint16_t target_id = target_y + target_x * node_count.y;

                if (addLink(zone, node_id, target_id))
                {
                    break;
                }

                if (target_x - 1 >= 0)
                {
                    target_id = target_y + (target_x - 1) * node_count.y;
                    addLink(zone, node_id, target_id);
                }

                if (target_x + 1 < node_count.y)
                {
                    target_id = target_y + (target_x + 1) * node_count.y;
                    addLink(zone, node_id, target_id);
                }
            }
        }
    }
}

bool Server::addLink(definitions::Zone& zone, uint16_t start, uint16_t finish)
{
    if (finish < start)
    {
        return false;
    }

    if (!getNodeById(zone, finish).has_value())
    {
        return false;
    }

    definitions::Zone::Link link;
    link.start = start;
    link.finish = finish;
    definitions::Zone::RegionNode start_node = getNodeById(zone, link.start).value();
    definitions::Zone::RegionNode finish_node = getNodeById(zone, link.finish).value();
    link.distance = util::Distance(start_node.coordinates, finish_node.coordinates) / 4;
    zone.links.push_back(link);

    return true;
}

void Server::gatherPlayers()
{
    bool gathered = true;
    for (auto& player : PlayerList)
    {
        if (!util::Intersects(player.GetBounds(), region.Convoy.GetInteriorBounds()))
        {
            gathered = false;
            break;
        }
    }

    if (gathered)
    {
        global::GatheringPlayers = false;
        global::Paused = true;
        global::RegionSelect = true;
        resetVotes();

        for (auto& p : PlayerList)
        {
            ServerMessage::SetGuiPause(*p.Socket, true, network::GuiType::Overmap);
        }
    }
}

void Server::resetVotes()
{
    for (auto& player : PlayerList)
    {
        player.Vote.voted = false;
        player.Vote.confirmed = false;
        player.Vote.vote = 0;
    }
}

void Server::checkVotes(VotingType voting_type)
{
    bool all_confirmed = true;
    for (auto& player : PlayerList)
    {
        if (!player.Vote.voted || !player.Vote.confirmed)
        {
            all_confirmed = false;
            break;
        }
    }

    if (!all_confirmed)
    {
        return;
    }

    std::map<uint8_t, int> vote_map;

    for (auto& player : PlayerList)
    {
        if (vote_map.find(player.Vote.vote) == vote_map.end())
        {
            vote_map[player.Vote.vote] = 1;
        }
        else
        {
            vote_map[player.Vote.vote] += 1;
        }
    }

    int winner = -1;
    int winning_count = 0;
    bool tie = true;
    for (auto& [vote, count] : vote_map)
    {
        if (count == winning_count)
        {
            tie = true;
        }
        else if (count > winning_count)
        {
            winner = vote;
            winning_count = count;
            tie = false;
        }
    }

    if (tie || winner < 0)
    {
        return;
    }

    switch (voting_type)
    {
        case VotingType::RegionSelect:
        {
            global::RegionSelect = false;
            global::Paused = false;
            next_region = winner;

            for (auto& p : PlayerList)
            {
                p.Status = Player::PlayerStatus::Loading;
                ServerMessage::SetGuiPause(*p.Socket, false, network::GuiType::Overmap);
                ServerMessage::ChangeRegion(*p.Socket, winner);
            }
        }
        break;
        case VotingType::MenuEvent:
        {
            resetVotes();
            uint16_t event_id;
            uint16_t event_action;
            if (region.AdvanceMenuEvent(static_cast<uint16_t>(winner), event_id, event_action))
            {
                resolveMenuEvent(event_id, event_action);
            }
        }
        break;
        default:
        {
            cerr << "Invalid voting type\n";
        }
    }
}

void Server::resolveMenuEvent(uint16_t event_id, uint16_t event_action)
{
    switch (static_cast<MenuEventId>(event_id))
    {
        case MenuEventId::DamageEvent:
        {
            switch (event_action)
            {
                case 0: // The water is cool and refreshing
                {
                    for (auto& player : PlayerList)
                    {
                        if (player.Data.health + 15 > 100)
                        {
                            player.Data.health = 100;
                        }
                        else
                        {
                            player.Data.health += 15;
                        }
                    }
                }
                break;
                case 1: // The sun is hot
                {
                    for (auto& player : PlayerList)
                    {
                        if (player.Data.health - 15 < 0)
                        {
                            player.Data.health = 0;
                        }
                        else
                        {
                            player.Data.health -= 15;
                        }
                    }
                }
                break;
            }
        }
        break;
        default:
        {
        }
    }
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

    current_zone = generateZone();

    for (auto& p : PlayerList)
    {
        if (p.Data.id != owner)
        {
            ServerMessage::StartGame(*p.Socket);
        }

        ServerMessage::SetZone(*p.Socket, current_zone);
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
        for (auto& link : current_zone.links)
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

        for (auto& node : current_zone.regions)
        {
            if (node.id == current_region)
            {
                region.~Region();
                new(&region)Region(node.type, PlayerList.size(), region.BatteryLevel - battery_cost);
                break;
            }
        }

        // Determine spawn positions
        float angle = 2 * util::pi / PlayerList.size();

        for (size_t i = 0; i < PlayerList.size(); ++i)
        {
            sf::Vector2f spawn_point = region.Convoy.Position;
            spawn_point.x += 200;
#ifndef NDEBUG
            if (debug::PlayerSpawnPoint.override)
            {
                spawn_point = debug::PlayerSpawnPoint.value;
            }
#endif
            // TODO: Magic numbers for distance
            sf::Vector2f spawn_position = sf::Vector2f(50 * std::sin(angle * i), 50 * std::cos(angle * i));
            spawn_position += spawn_point;
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

    bool permitted = true;
    if (action.type == network::PlayerActionType::Attack)
    {
        permitted = player.StartAttack(action.action_angle);
    }

    if (permitted)
    {
        for (auto& p : PlayerList)
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

void Server::castVote(Player& player)
{
    uint8_t vote;
    bool confirm;
    if (!ClientMessage::DecodeCastVote(*player.Socket, vote, confirm))
    {
        player.Socket->disconnect();
        player.Status = Player::PlayerStatus::Disconnected;
    }

    if (!(global::RegionSelect || global::MenuEvent) /* TODO: Add check for menu events here */)
    {
        cerr << "A vote was casting during a non-voting period\n";
        return;
    }

    player.Vote.voted = true;
    player.Vote.vote = vote;
    player.Vote.confirmed = confirm;

    for (auto& p : PlayerList)
    {
        ServerMessage::CastVote(*p.Socket, player.Data.id, vote, confirm);
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

    global::GatheringPlayers = activate;

    for (auto& p : PlayerList)
    {
        ServerMessage::GatherPlayers(*p.Socket, player.Data.id, global::GatheringPlayers);
    }

    if (!activate)
    {
        global::Paused = false;
        for (auto& p : PlayerList)
        {
            ServerMessage::SetGuiPause(*p.Socket, false, network::GuiType::Overmap);
        }
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
        enemy_list.push_back(enemy.GetData());
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
