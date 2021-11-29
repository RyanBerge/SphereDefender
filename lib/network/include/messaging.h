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

namespace network {

struct PlayerAction
{
    bool start_attack;
    uint16_t attack_angle;
};

struct EnemyAction
{
    bool start_attack;
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

        StartGame,
        LoadingComplete,

        PlayerStateChange,
        StartAction,

        LeaveGame,

        Error = 0xFF
    };

    static bool PollForCode(sf::TcpSocket& socket, Code& out_code);

    static bool InitLobby(sf::TcpSocket& socket, std::string name);
    static bool JoinLobby(sf::TcpSocket& socket, std::string name);
    static bool StartGame(sf::TcpSocket& socket);
    static bool LoadingComplete(sf::TcpSocket& socket);
    static bool LeaveGame(sf::TcpSocket& socket);
    static bool PlayerStateChange(sf::TcpSocket& socket, sf::Vector2i movement_vector);
    static bool StartAction(sf::TcpSocket& socket, PlayerAction action);

    static bool DecodeInitLobby(sf::TcpSocket& socket, std::string& out_name);
    static bool DecodeJoinLobby(sf::TcpSocket& socket, std::string& out_name);
    static bool DecodePlayerStateChange(sf::TcpSocket& socket, sf::Vector2i& out_movement_vector);
    static bool DecodeStartAction(sf::TcpSocket& socket, PlayerAction& out_action);
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

        StartGame,
        AllPlayersLoaded,

        RegionInfo,

        PlayerStates,
        PlayerStartAction,
        EnemyStartAction,
        EnemyUpdate,

        Error = 0xFF
    };

    static bool PollForCode(sf::TcpSocket& socket, Code& out_code);

    static bool PlayerId(sf::TcpSocket& socket, uint16_t player_id);
    static bool PlayerJoined(sf::TcpSocket& socket, PlayerData player);
    static bool PlayerLeft(sf::TcpSocket& socket, uint16_t player_id);
    static bool PlayersInLobby(sf::TcpSocket& socket, uint16_t player_id, std::vector<PlayerData> players);
    static bool OwnerLeft(sf::TcpSocket& socket);
    static bool StartGame(sf::TcpSocket& socket);
    static bool AllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f spawn_position);
    static bool PlayerStates(sf::TcpSocket& socket, std::vector<PlayerData> players);
    static bool PlayerStartAction(sf::TcpSocket& socket, uint16_t player_id, PlayerAction action);
    static bool EnemyStartAction(sf::TcpSocket& socket, uint16_t player_id, EnemyAction action);
    static bool RegionInfo(sf::TcpSocket& socket, ConvoyData convoy, std::vector<EnemyData> enemies);
    static bool EnemyUpdate(sf::TcpSocket& socket, std::vector<EnemyData> enemies);

    static bool DecodePlayerId(sf::TcpSocket& socket, uint16_t& out_id);
    static bool DecodePlayerJoined(sf::TcpSocket& socket, PlayerData& out_player);
    static bool DecodePlayerLeft(sf::TcpSocket& socket, uint16_t& out_id);
    static bool DecodePlayersInLobby(sf::TcpSocket& socket, uint16_t& out_id, std::vector<PlayerData>& out_players);
    static bool DecodeAllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f& out_spawn_position);
    static bool DecodePlayerStates(sf::TcpSocket& socket, std::vector<PlayerData>& out_players);
    static bool DecodePlayerStartAction(sf::TcpSocket& socket, uint16_t& out_player_id, PlayerAction& out_action);
    static bool DecodeEnemyStartAction(sf::TcpSocket& socket, uint16_t& out_player_id, EnemyAction& out_action);
    static bool DecodeRegionInfo(sf::TcpSocket& socket, ConvoyData& out_convoy, std::vector<EnemyData>& out_enemies);
    static bool DecodeEnemyUpdate(sf::TcpSocket& socket, std::vector<EnemyData>& out_enemies);
};

} // network
