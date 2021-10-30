#pragma once
#include <SFML/Network/TcpSocket.hpp>
#include <string>

namespace Network
{
    enum class ClientMessage : uint8_t
    {
        Error = 0,
        InitServer,
        JoinServer,

        StartGame,
        LoadingComplete,

        LeaveGame
    };

    enum class ServerMessage : uint8_t
    {
        Error = 0,
        PlayerId,
        PlayerJoined,
        PlayerLeft,
        OwnerLeft,
        PlayersInLobby,

        StartGame,
        AllPlayersLoaded
    };

    extern const uint32_t ServerPort;

    bool Read(sf::TcpSocket& socket, void* buf, int num_bytes);
    std::string ReadString(sf::TcpSocket& socket);

    bool Write(sf::TcpSocket& socket, const void* buf, int num_bytes);
    bool WriteString(sf::TcpSocket& socket, const std::string& str);

    int CopyStringToBuffer(uint8_t* buffer, int offset, std::string data);
}
