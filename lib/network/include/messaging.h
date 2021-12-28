/**************************************************************************************************
 *  File:       messaging.h
 *
 *  Purpose:    Functions for handling the messaging protocols to and from the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include "entity_data.h"
#include "region_definitions.h"

namespace network {

struct PlayerActionFlags
{
    bool start_attack: 1;
};

struct EnemyActionFlags
{
    bool move: 1;
    bool feed: 1;
    bool knockback: 1;
    bool stunned: 1;
    bool start_attack: 1;
    bool dead: 1;
    bool sniffing: 1;
    bool leaping: 1;
};

struct PlayerAction
{
    PlayerActionFlags flags;
    uint16_t attack_angle;
};

struct EnemyAction
{
    EnemyActionFlags flags;
    sf::Vector2f attack_vector;
};

class ClientMessage
{
public:
    enum class Code : uint8_t
    {
        None = 0,
        InitLobby,
        JoinLobby,
        ChangePlayerProperty,

        StartGame,
        LoadingComplete,

        PlayerStateChange,
        StartAction,
        UseItem,
        ChangeRegion,

        LeaveGame,

        Error = 0xFF
    };

    static bool PollForCode(sf::TcpSocket& socket, Code& out_code);

    static bool InitLobby(sf::TcpSocket& socket, std::string name);
    static bool JoinLobby(sf::TcpSocket& socket, std::string name);
    static bool ChangePlayerProperty(sf::TcpSocket& socket, PlayerProperties properties);
    static bool StartGame(sf::TcpSocket& socket);
    static bool LoadingComplete(sf::TcpSocket& socket);
    static bool LeaveGame(sf::TcpSocket& socket);
    static bool PlayerStateChange(sf::TcpSocket& socket, sf::Vector2i movement_vector);
    static bool StartAction(sf::TcpSocket& socket, PlayerAction action);
    static bool UseItem(sf::TcpSocket& socket);
    static bool ChangeRegion(sf::TcpSocket& socket, definitions::RegionName region);

    static bool DecodeInitLobby(sf::TcpSocket& socket, std::string& out_name);
    static bool DecodeJoinLobby(sf::TcpSocket& socket, std::string& out_name);
    static bool DecodeChangePlayerProperty(sf::TcpSocket& socket, PlayerProperties& out_properties);
    static bool DecodePlayerStateChange(sf::TcpSocket& socket, sf::Vector2i& out_movement_vector);
    static bool DecodeStartAction(sf::TcpSocket& socket, PlayerAction& out_action);
    static bool DecodeChangeRegion(sf::TcpSocket& socket, definitions::RegionName&  out_region);
};

class ServerMessage
{
public:
    enum class Code : uint8_t
    {
        None = 0,
        PlayerId,
        PlayerJoined,
        PlayerLeft,
        OwnerLeft,
        PlayersInLobby,
        ChangePlayerProperty,

        StartGame,
        AllPlayersLoaded,

        RegionInfo,

        PlayerStartAction,
        EnemyChangeAction,
        ChangeItem,
        PlayerStates,
        EnemyUpdate,
        BatteryUpdate,
        ProjectileUpdate,
        ChangeRegion,

        Error = 0xFF
    };

    static bool PollForCode(sf::TcpSocket& socket, Code& out_code);

    static bool PlayerId(sf::TcpSocket& socket, uint16_t player_id);
    static bool PlayerJoined(sf::TcpSocket& socket, PlayerData player);
    static bool PlayerLeft(sf::TcpSocket& socket, uint16_t player_id);
    static bool PlayersInLobby(sf::TcpSocket& socket, uint16_t player_id, std::vector<PlayerData> players);
    static bool ChangePlayerProperty(sf::TcpSocket& socket, uint16_t player_id, PlayerProperties properties);
    static bool OwnerLeft(sf::TcpSocket& socket);
    static bool StartGame(sf::TcpSocket& socket);
    static bool AllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f spawn_position);
    static bool PlayerStartAction(sf::TcpSocket& socket, uint16_t player_id, PlayerAction action);
    static bool EnemyChangeAction(sf::TcpSocket& socket, uint16_t enemy_id, EnemyAction action);
    static bool ChangeItem(sf::TcpSocket& socket, definitions::ItemType item);
    static bool PlayerStates(sf::TcpSocket& socket, std::vector<PlayerData> players);
    static bool EnemyUpdate(sf::TcpSocket& socket, std::vector<EnemyData> enemies);
    static bool BatteryUpdate(sf::TcpSocket& socket, float battery_level);
    static bool ProjectileUpdate(sf::TcpSocket& socket, std::vector<ProjectileData> projectiles);
    static bool ChangeRegion(sf::TcpSocket& socket, definitions::RegionName region);

    static bool DecodePlayerId(sf::TcpSocket& socket, uint16_t& out_id);
    static bool DecodePlayerJoined(sf::TcpSocket& socket, PlayerData& out_player);
    static bool DecodePlayerLeft(sf::TcpSocket& socket, uint16_t& out_id);
    static bool DecodePlayersInLobby(sf::TcpSocket& socket, uint16_t& out_id, std::vector<PlayerData>& out_players);
    static bool DecodeChangePlayerProperty(sf::TcpSocket& socket, uint16_t& out_player_id, PlayerProperties& out_properties);
    static bool DecodeAllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f& out_spawn_position);
    static bool DecodePlayerStartAction(sf::TcpSocket& socket, uint16_t& out_player_id, PlayerAction& out_action);
    static bool DecodeEnemyChangeAction(sf::TcpSocket& socket, uint16_t& out_enemy_id, EnemyAction& out_action);
    static bool DecodeChangeItem(sf::TcpSocket& socket, definitions::ItemType& out_item);
    static bool DecodePlayerStates(sf::TcpSocket& socket, std::vector<PlayerData>& out_players);
    static bool DecodeEnemyUpdate(sf::TcpSocket& socket, std::vector<EnemyData>& out_enemies);
    static bool DecodeBatteryUpdate(sf::TcpSocket& socket, float& out_battery_level);
    static bool DecodeProjectileUpdate(sf::TcpSocket& socket, std::vector<ProjectileData>& out_projectiles);
    static bool DecodeChangeRegion(sf::TcpSocket& socket, definitions::RegionName& out_region);
};

} // network
