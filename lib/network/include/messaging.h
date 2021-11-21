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

namespace network {

struct PlayerData
{
    uint16_t id;
    std::string name;
    sf::Vector2f position;
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

        LeaveGame,

        Error = 0xFF
    };

    struct MovementVectorField
    {
        bool left: 1;
        bool right: 1;
        bool up: 1;
        bool down: 1;
    };

    static bool PollForCode(sf::TcpSocket& socket, Code& out_code);

    static bool InitLobby(sf::TcpSocket& socket, std::string name);
    static bool JoinLobby(sf::TcpSocket& socket, std::string name);
    static bool StartGame(sf::TcpSocket& socket);
    static bool LoadingComplete(sf::TcpSocket& socket);
    static bool LeaveGame(sf::TcpSocket& socket);
    static bool PlayerStateChange(sf::TcpSocket& socket, sf::Vector2i movement_vector);

    static bool DecodeInitLobby(sf::TcpSocket& socket, std::string& out_name);
    static bool DecodeJoinLobby(sf::TcpSocket& socket, std::string& out_name);
    static bool DecodePlayerStateChange(sf::TcpSocket& socket, sf::Vector2i& out_movement_vector);
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

        PlayerStates,

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

    static bool DecodePlayerId(sf::TcpSocket& socket, uint16_t& out_id);
    static bool DecodePlayerJoined(sf::TcpSocket& socket, PlayerData& out_player);
    static bool DecodePlayerLeft(sf::TcpSocket& socket, uint16_t& out_id);
    static bool DecodePlayersInLobby(sf::TcpSocket& socket, uint16_t& out_id, std::vector<PlayerData>& out_players);
    static bool DecodeAllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f& out_spawn_position);
    static bool DecodePlayerStates(sf::TcpSocket& socket, std::vector<PlayerData>& out_players);
};

} // network
