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

struct MovementVectorFlags
{
    bool left: 1;
    bool right: 1;
    bool up: 1;
    bool down: 1;
};

struct PlayerActionFlags
{
    bool start_attack: 1;
};

// DEBUG
[[maybe_unused]] void printBuffer(uint8_t* buffer, int length)
{
    cout << "| ";
    for (int i = 0; i < length; ++i)
    {
        cout << (int)buffer[i] << " | ";
    }
    cout << endl;
}

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

bool ClientMessage::InitLobby(sf::TcpSocket& socket, std::string name)
{
    Code code = Code::InitLobby;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = sizeof(code) + sizeof(str_len) + str_len;
    uint8_t* buffer = new uint8_t[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &str_len, 2);
    std::memcpy(buffer + sizeof(code) + sizeof(str_len), name.c_str(), str_len);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ClientMessage::InitLobby message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::JoinLobby(sf::TcpSocket& socket, std::string name)
{
    Code code = Code::JoinLobby;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = sizeof(code) + sizeof(str_len) + str_len;
    uint8_t* buffer = new uint8_t[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &str_len, 2);
    std::memcpy(buffer + sizeof(code) + sizeof(str_len), name.c_str(), str_len);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ClientMessage::JoinLobby message" << endl;
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

bool ClientMessage::PlayerStateChange(sf::TcpSocket& socket, sf::Vector2i movement_vector)
{
    Code code = Code::PlayerStateChange;

    MovementVectorFlags flags{false, false, false, false};

    if (movement_vector.x > 0)
    {
        flags.right = true;
    }
    else if (movement_vector.x < 0)
    {
        flags.left = true;
    }

    if (movement_vector.y > 0)
    {
        flags.down = true;
    }
    else if (movement_vector.y < 0)
    {
        flags.up = true;
    }

    constexpr size_t buffer_size = sizeof(code) + sizeof(flags);
    uint8_t buffer[buffer_size];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &flags, sizeof(flags));

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::PlayerState message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::StartAction(sf::TcpSocket& socket, PlayerAction action)
{
    Code code = ClientMessage::Code::StartAction;

    PlayerActionFlags flags{};
    flags.start_attack = action.start_attack;

    size_t buffer_size = sizeof(code) + sizeof(flags);
    if (flags.start_attack)
    {
        buffer_size += sizeof(action.attack_angle);
    }

    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &flags, sizeof(flags));
    offset += sizeof(flags);

    if (flags.start_attack)
    {
        std::memcpy(buffer + offset, &action.attack_angle, sizeof(action.attack_angle));
        offset += sizeof(action.attack_angle);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::StartAction message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::DecodeInitLobby(sf::TcpSocket& socket, std::string& out_name)
{
    std::string name;
    if (!readString(socket, name))
    {
        cerr << "Network: DecodeInitLobby failed to read a player name." << endl;
        return false;
    }

    out_name = name;
    return true;
}

bool ClientMessage::DecodeJoinLobby(sf::TcpSocket& socket, std::string& out_name)
{
    std::string name;
    if (!readString(socket, name))
    {
        cerr << "Network: DecodeJoinLobby failed to read a player name." << endl;
        return false;
    }

    out_name = name;
    return true;
}

bool ClientMessage::DecodePlayerStateChange(sf::TcpSocket& socket, sf::Vector2i& out_movement_vector)
{
    MovementVectorFlags flags;

    if (!read(socket, &flags, sizeof(flags)))
    {
        cerr << "Network: DecodePlayerStateChange failed to read spawn x position." << endl;
        return false;
    }

    out_movement_vector.x = 0;
    out_movement_vector.y = 0;

    if (flags.left)
    {
        --out_movement_vector.x;
    }

    if (flags.right)
    {
        ++out_movement_vector.x;
    }

    if (flags.up)
    {
        --out_movement_vector.y;
    }

    if (flags.down)
    {
        ++out_movement_vector.y;
    }

    return true;
}

bool ClientMessage::DecodeStartAction(sf::TcpSocket& socket, PlayerAction& out_action)
{
    PlayerActionFlags flags;
    PlayerAction temp_action;

    if (!read(socket, &flags, sizeof(flags)))
    {
        cerr << "Network: DecodeStartAction failed to read player action flags." << endl;
        return false;
    }

    temp_action.start_attack = flags.start_attack;

    if (flags.start_attack)
    {
        if (!read(socket, &temp_action.attack_angle, sizeof(temp_action.attack_angle)))
        {
            cerr << "Network: DecodeStartAction failed to read an attack angle." << endl;
            return false;
        }
    }

    out_action = temp_action;

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

    constexpr size_t buffer_len = sizeof(code) + sizeof(player_id);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &player_id, sizeof(player_id));

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::PlayerId message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::PlayerJoined(sf::TcpSocket& socket, PlayerData player)
{
    Code code = Code::PlayerJoined;
    uint16_t str_len = player.name.size();

    // code + len + string
    size_t buffer_len = sizeof(code) + sizeof(player.id) + sizeof(str_len) + str_len;
    uint8_t* buffer = new uint8_t[buffer_len];

    int offset = 0;

    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player.id, sizeof(player.id));
    offset += sizeof(player.id);
    std::memcpy(buffer + offset, &str_len, sizeof(str_len));
    offset += sizeof(str_len);
    std::memcpy(buffer + offset, player.name.c_str(), str_len);

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

    constexpr size_t buffer_len = sizeof(code) + sizeof(player_id);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &player_id, sizeof(player_id));

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::PlayerLeft message" << endl;
        return false;
    }

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
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
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

bool ServerMessage::AllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f spawn_position)
{
    Code code = Code::AllPlayersLoaded;

    constexpr size_t buffer_size = sizeof(code) + sizeof(spawn_position.x) + sizeof(spawn_position.y);
    uint8_t buffer[buffer_size];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &spawn_position.x, sizeof(spawn_position.x));
    std::memcpy(buffer + sizeof(code) + sizeof(spawn_position.x), &spawn_position.y, sizeof(spawn_position.y));

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::AllPlayersLoaded message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::PlayerStates(sf::TcpSocket& socket, std::vector<PlayerData> players)
{
    Code code = Code::PlayerStates;

    size_t buffer_size = sizeof(code) + ((sizeof(uint16_t) + sizeof(float) * 2) * players.size());
    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);

    for (auto& player : players)
    {
        std::memcpy(buffer + offset, &player.id, sizeof(player.id));
        offset += sizeof(player.id);
        std::memcpy(buffer + offset, &player.position.x, sizeof(player.position.x));
        offset += sizeof(player.position.x);
        std::memcpy(buffer + offset, &player.position.y, sizeof(player.position.y));
        offset += sizeof(player.position.y);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::AllPlayersLoaded message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::StartAction(sf::TcpSocket& socket, uint16_t player_id, PlayerAction action)
{
    Code code = ServerMessage::Code::StartAction;

    PlayerActionFlags flags{};
    flags.start_attack = action.start_attack;

    size_t buffer_size = sizeof(code) + sizeof(player_id) + sizeof(flags);
    if (flags.start_attack)
    {
        buffer_size += sizeof(action.attack_angle);
    }

    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &flags, sizeof(flags));
    offset += sizeof(flags);

    if (flags.start_attack)
    {
        std::memcpy(buffer + offset, &action.attack_angle, sizeof(action.attack_angle));
        offset += sizeof(action.attack_angle);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::StartAction message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
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

bool ServerMessage::DecodeAllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f& out_spawn_position)
{
    float x;
    float y;

    if (!read(socket, &x, sizeof(x)))
    {
        cerr << "Network: DecodeAllPlayersLoaded failed to read a spawn x position." << endl;
        return false;
    }

    if (!read(socket, &y, sizeof(y)))
    {
        cerr << "Network: DecodeAllPlayersLoaded failed to read a spawn y position." << endl;
        return false;
    }

    out_spawn_position.x = x;
    out_spawn_position.y = y;

    return true;
}

bool ServerMessage::DecodePlayerStates(sf::TcpSocket& socket, std::vector<PlayerData>& out_players)
{
    std::vector<PlayerData> players = out_players;

    for (size_t i = 0; i < players.size(); ++i)
    {
        if (!read(socket, &players[i].id, sizeof(players[i].id)))
        {
            cerr << "Network: DecodePlayerStates failed to read a player id." << endl;
            return false;
        }

        if (!read(socket, &players[i].position.x, sizeof(players[i].position.x)))
        {
            cerr << "Network: DecodePlayerStates failed to read an x position." << endl;
            return false;
        }

        if (!read(socket, &players[i].position.y, sizeof(players[i].position.y)))
        {
            cerr << "Network: DecodePlayerStates failed to read a y position." << endl;
            return false;
        }
    }

    out_players = players;

    return true;
}

bool ServerMessage::DecodeStartAction(sf::TcpSocket& socket, uint16_t& out_player_id, PlayerAction& out_action)
{
    uint16_t id;
    PlayerActionFlags flags;
    PlayerAction temp_action;

    if (!read(socket, &id, sizeof(id)))
    {
        cerr << "Network: DecodeStartAction failed to read player id." << endl;
        return false;
    }

    if (!read(socket, &flags, sizeof(flags)))
    {
        cerr << "Network: DecodeStartAction failed to read player action flags." << endl;
        return false;
    }

    temp_action.start_attack = flags.start_attack;

    if (flags.start_attack)
    {
        if (!read(socket, &temp_action.attack_angle, sizeof(temp_action.attack_angle)))
        {
            cerr << "Network: DecodeStartAction failed to read an attack angle." << endl;
            return false;
        }
    }

    out_player_id = id;
    out_action = temp_action;

    return true;
}

} // server
