/**************************************************************************************************
 *  File:       messaging.h
 *
 *  Purpose:    Functions for handling the messaging protocols to and from the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "messaging.h"
#include <SFML/System/Clock.hpp>
#include <cstring>
#include <iostream>

using std::cout, std::cerr, std::endl;

namespace network {

namespace {

const int SOCKET_TIMEOUT_MS = 300;

bool writeBuffer(sf::TcpSocket& socket, const void* data, int num_bytes)
{
    size_t bytes_sent = 0;
    auto status = socket.send(data, num_bytes, bytes_sent);

    size_t offset = bytes_sent;
    while (status == sf::Socket::Partial)
    {
        status = socket.send(&reinterpret_cast<const uint8_t*>(data)[offset], num_bytes - offset, bytes_sent);
        offset += bytes_sent;
    }

    return (status == sf::Socket::Done);
}

bool read(sf::TcpSocket& socket, void* out_buffer, int num_bytes, int timeout = SOCKET_TIMEOUT_MS)
{
    uint8_t* buffer = reinterpret_cast<uint8_t*>(out_buffer);

    sf::Clock time;

    int bytes_read = 0;
    while (bytes_read < num_bytes)
    {
        std::size_t b_read;
        auto status = socket.receive(&buffer[bytes_read], num_bytes - bytes_read, b_read);

        if (status == sf::Socket::Error)
        {
            // This really should never happen
            cerr << "Network: An unrecognized socket error occurred." << endl;
            return false;
        }
        else if (status == sf::Socket::Disconnected)
        {
            cerr << "Closed socket detected." << endl;
            return false;
        }
        else if (status == sf::Socket::NotReady)
        {
            if (timeout == 0)
            {
                break;
            }

            if (time.getElapsedTime().asMilliseconds() > timeout)
            {
                cerr << "Socket read exceed timeout value of " << timeout << " milliseconds." << endl;
                return false;
            }
        }

        bytes_read += b_read;
        buffer += bytes_read;
    }

    return true;
}

bool readString(sf::TcpSocket& socket, std::string& out_string)
{
    uint16_t string_size;

    if (!read(socket, &string_size, 2))
    {
        return false;
    }

    char* name_buffer = new char[string_size];
    if (!read(socket, name_buffer, string_size))
    {
        delete[] name_buffer;
        return false;
    }

    out_string = std::string{name_buffer, string_size};
    delete[] name_buffer;
    return true;
}

} // anonymous namespace

// ====================================================== Client Message ======================================================

bool ClientMessage::PollForCode(sf::TcpSocket& socket, Code& out_code)
{
    Code code = Code::None;
    if (!read(socket, &code, sizeof(code), 0))
    {
        cerr << "Network: ClientMessage::PollForCode encountered a socket error." << endl;
        return false;
    }

    out_code = code;
    return true;
}

bool ClientMessage::InitServer(sf::TcpSocket& socket, std::string name)
{
    Code code = Code::InitServer;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = sizeof(code) + sizeof(str_len) + str_len;
    uint8_t* buffer = new uint8_t[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &str_len, 2);
    std::memcpy(buffer + sizeof(code) + sizeof(str_len), name.c_str(), str_len);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ClientMessage::InitServer message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::JoinServer(sf::TcpSocket& socket, std::string name)
{
    Code code = Code::JoinServer;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = sizeof(code) + sizeof(str_len) + str_len;
    uint8_t* buffer = new uint8_t[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &str_len, 2);
    std::memcpy(buffer + sizeof(code) + sizeof(str_len), name.c_str(), str_len);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ClientMessage::JoinServer message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::StartGame(sf::TcpSocket& socket)
{
    Code code = Code::StartGame;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::StartGame message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::LoadingComplete(sf::TcpSocket& socket)
{
    Code code = Code::LoadingComplete;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::LoadingComplete message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::LeaveGame(sf::TcpSocket& socket)
{
    Code code = Code::LeaveGame;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::LeaveGame message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::DecodeInitServer(sf::TcpSocket& socket, std::string& out_name)
{
    std::string name;
    if (!readString(socket, name))
    {
        cerr << "Network: DecodeInitServer failed to read a player name." << endl;
        return false;
    }

    out_name = name;
    return true;
}

bool ClientMessage::DecodeJoinServer(sf::TcpSocket& socket, std::string& out_name)
{
    std::string name;
    if (!readString(socket, name))
    {
        cerr << "Network: DecodeJoinServer failed to read a player name." << endl;
        return false;
    }

    out_name = name;
    return true;
}

// ====================================================== Server Message ======================================================

bool ServerMessage::PollForCode(sf::TcpSocket& socket, Code& out_code)
{
    Code code = Code::None;
    if (!read(socket, &code, sizeof(code), 0))
    {
        cerr << "Network: ClientMessage::PollForCode encountered a socket error." << endl;
        return false;
    }

    out_code = code;
    return true;
}

bool ServerMessage::PlayerId(sf::TcpSocket& socket, uint16_t player_id)
{
    Code code = Code::PlayerId;

    size_t buffer_len = sizeof(code) + sizeof(player_id);
    uint8_t* buffer = new uint8_t[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &player_id, sizeof(player_id));

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::PlayerId message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::PlayerJoined(sf::TcpSocket& socket, std::string name, uint16_t player_id)
{
    Code code = Code::PlayerJoined;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = sizeof(code) + sizeof(player_id) + sizeof(str_len) + str_len;
    uint8_t* buffer = new uint8_t[buffer_len];

    int offset = 0;

    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &str_len, sizeof(str_len));
    offset += sizeof(str_len);
    std::memcpy(buffer + offset, name.c_str(), str_len);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::PlayerJoined message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::PlayerLeft(sf::TcpSocket& socket, uint16_t player_id)
{
    Code code = Code::PlayerLeft;

    size_t buffer_len = sizeof(code) + sizeof(player_id);
    uint8_t* buffer = new uint8_t[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &player_id, sizeof(player_id));

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::PlayerLeft message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::PlayersInLobby(sf::TcpSocket& socket, uint16_t player_id, std::vector<PlayerData> players)
{
    Code code = Code::PlayersInLobby;

    uint8_t num_players = players.size();
    size_t buffer_len = sizeof(code) + sizeof(player_id) + sizeof(num_players);

    for (auto& player : players)
    {
        buffer_len += sizeof(player.id);
        buffer_len += 2;
        buffer_len += player.name.size();
    }

    uint8_t* buffer = new uint8_t[buffer_len];

    int offset = 0;

    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &num_players, sizeof(num_players));
    offset += sizeof(num_players);

    for (auto& player : players)
    {
        uint16_t str_len = player.name.size();

        std::memcpy(buffer + offset, &player.id, sizeof(player.id));
        offset += sizeof(player.id);
        std::memcpy(buffer + offset, &str_len, sizeof(str_len));
        offset += sizeof(str_len);
        std::memcpy(buffer + offset, player.name.c_str(), str_len);
        offset += str_len;
    }

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::PlayersInLobby message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::OwnerLeft(sf::TcpSocket& socket)
{
    Code code = Code::OwnerLeft;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ServerMessage::OwnerLeft message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::StartGame(sf::TcpSocket& socket)
{
    Code code = Code::StartGame;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ServerMessage::StartGame message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::AllPlayersLoaded(sf::TcpSocket& socket)
{
    Code code = Code::AllPlayersLoaded;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ServerMessage::AllPlayersLoaded message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::DecodePlayerId(sf::TcpSocket& socket, uint16_t& out_id)
{
    uint16_t id;
    if (!read(socket, &id, sizeof(id)))
    {
        cerr << "Network: DecodePlayerId failed to read a player id." << endl;
        return false;
    }

    out_id = id;
    return true;
}

bool ServerMessage::DecodePlayerJoined(sf::TcpSocket& socket, PlayerData& out_player)
{
    uint16_t id;
    std::string name;

    if (!read(socket, &id, sizeof(id)))
    {
        cerr << "Network: DecodePlayerJoined failed to read a player id." << endl;
        return false;
    }

    if (!readString(socket, name))
    {
        cerr << "Network: DecodePlayerJoined failed to read a player name." << endl;
        return false;
    }

    out_player.id = id;
    out_player.name = name;
    return true;
}

bool ServerMessage::DecodePlayerLeft(sf::TcpSocket& socket, uint16_t& out_id)
{
    uint16_t id;
    if (!read(socket, &id, sizeof(id)))
    {
        cerr << "Network: DecodePlayerLeft failed to read a player id." << endl;
        return false;
    }

    out_id = id;
    return true;
}

bool ServerMessage::DecodePlayersInLobby(sf::TcpSocket& socket, uint16_t& out_id, std::vector<PlayerData>& out_players)
{
    uint8_t num_players;
    uint16_t player_id;
    std::vector<PlayerData> players;

    if (!read(socket, &player_id, sizeof(player_id)))
    {
        cerr << "Network: DecodePlayersInLobby failed to read a player id." << endl;
        return false;
    }

    if (!read(socket, &num_players, sizeof(num_players)))
    {
        cerr << "Network: DecodePlayersInLobby failed to read a player count." << endl;
        return false;
    }

    for (int i = 0; i < num_players; ++i)
    {
        PlayerData data{};

        if (!read(socket, &data.id, sizeof(data.id)))
        {
            cerr << "Network: DecodePlayersInLobby failed to read a player id." << endl;
            return false;
        }

        if (!readString(socket, data.name))
        {
            cerr << "Network: DecodePlayersInLobby failed to read a player name." << endl;
            return false;
        }

        players.push_back(data);
    }

    out_players = players;
    out_id = player_id;

    return true;
}


} // server
