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
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
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
    std::memcpy(buffer + sizeof(code), &str_len, sizeof(str_len));
    std::memcpy(buffer + sizeof(code) + sizeof(str_len), name.c_str(), str_len);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::ChangePlayerProperty(sf::TcpSocket& socket, PlayerProperties properties)
{
    Code code = Code::ChangePlayerProperty;

    constexpr size_t buffer_len = sizeof(code) + sizeof(properties);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + sizeof(code), &properties, sizeof(properties));

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::StartGame(sf::TcpSocket& socket)
{
    Code code = Code::StartGame;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::LoadingComplete(sf::TcpSocket& socket)
{
    Code code = Code::LoadingComplete;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::LeaveGame(sf::TcpSocket& socket)
{
    Code code = Code::LeaveGame;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
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
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::StartAction(sf::TcpSocket& socket, PlayerAction action)
{
    Code code = ClientMessage::Code::StartAction;

    size_t buffer_size = sizeof(code) + sizeof(action.flags);
    if (action.flags.start_attack)
    {
        buffer_size += sizeof(action.attack_angle);
    }

    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &action.flags, sizeof(action.flags));
    offset += sizeof(action.flags);

    if (action.flags.start_attack)
    {
        std::memcpy(buffer + offset, &action.attack_angle, sizeof(action.attack_angle));
        offset += sizeof(action.attack_angle);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::UseItem(sf::TcpSocket& socket)
{
    Code code = ClientMessage::Code::UseItem;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::SwapItem(sf::TcpSocket& socket, uint8_t item_index)
{
    Code code = ClientMessage::Code::SwapItem;

    constexpr size_t buffer_size = sizeof(code) + sizeof(item_index);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &item_index, sizeof(item_index));
    offset += sizeof(item_index);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::CastVote(sf::TcpSocket& socket, uint8_t vote, bool confirm)
{
    Code code = ClientMessage::Code::CastVote;

    constexpr size_t buffer_size = sizeof(code) + sizeof(vote) + sizeof(confirm);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &vote, sizeof(vote));
    offset += sizeof(vote);
    std::memcpy(buffer + offset, &confirm, sizeof(confirm));
    offset += sizeof(confirm);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ClientMessage::Console(sf::TcpSocket& socket, bool activate)
{
    Code code = ClientMessage::Code::Console;

    constexpr size_t buffer_size = sizeof(code) + sizeof(activate);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &activate, sizeof(activate));
    offset += sizeof(activate);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

//bool ClientMessage::ChangeRegion(sf::TcpSocket& socket, uint16_t region_id)
//{
//    Code code = ClientMessage::Code::ChangeRegion;
//
//    constexpr size_t buffer_size = sizeof(code) + sizeof(region_id);
//    uint8_t buffer[buffer_size];
//
//    int offset = 0;
//    std::memcpy(buffer, &code, sizeof(code));
//    offset += sizeof(code);
//    std::memcpy(buffer + offset, &region_id, sizeof(region_id));
//    offset += sizeof(region_id);
//
//    if (!writeBuffer(socket, buffer, buffer_size))
//    {
//        cerr << "Network: Failed to send ClientMessage::" << __func__ << " message" << endl;
//        return false;
//    }
//
//    return true;
//}

bool ClientMessage::DecodeInitLobby(sf::TcpSocket& socket, std::string& out_name)
{
    std::string name;
    if (!readString(socket, name))
    {
        cerr << "Network: " << __func__ << " failed to read a player name." << endl;
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
        cerr << "Network: " << __func__ << " failed to read a player name." << endl;
        return false;
    }

    out_name = name;
    return true;
}

bool ClientMessage::DecodeChangePlayerProperty(sf::TcpSocket& socket, PlayerProperties& out_properties)
{
    PlayerProperties properties;
    if (!read(socket, &properties, sizeof(properties)))
    {
        cerr << "Network: " << __func__ << " failed to read player properties." << endl;
        return false;
    }

    out_properties = properties;
    return true;
}

bool ClientMessage::DecodePlayerStateChange(sf::TcpSocket& socket, sf::Vector2i& out_movement_vector)
{
    MovementVectorFlags flags;

    if (!read(socket, &flags, sizeof(flags)))
    {
        cerr << "Network: " << __func__ << " failed to read spawn x position." << endl;
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
    PlayerAction temp_action;

    if (!read(socket, &temp_action.flags, sizeof(temp_action.flags)))
    {
        cerr << "Network: " << __func__ << " failed to read player action flags." << endl;
        return false;
    }

    if (temp_action.flags.start_attack)
    {
        if (!read(socket, &temp_action.attack_angle, sizeof(temp_action.attack_angle)))
        {
            cerr << "Network: " << __func__ << " failed to read an attack angle." << endl;
            return false;
        }
    }

    out_action = temp_action;

    return true;
}

bool ClientMessage::DecodeSwapItem(sf::TcpSocket& socket, uint8_t& out_item_index)
{
    uint8_t item_index;

    if (!read(socket, &item_index, sizeof(item_index)))
    {
        cerr << "Network: " << __func__ << " failed to read player action flags." << endl;
        return false;
    }

    out_item_index = item_index;
    return true;
}

bool ClientMessage::DecodeCastVote(sf::TcpSocket& socket, uint8_t& out_vote, bool& out_confirm)
{
    uint8_t vote;
    bool confirm;

    if (!read(socket, &vote, sizeof(vote)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    if (!read(socket, &confirm, sizeof(confirm)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    out_vote = vote;
    out_confirm = confirm;
    return true;
}

bool ClientMessage::DecodeConsole(sf::TcpSocket& socket, bool& out_activate)
{
    bool activate;

    if (!read(socket, &activate, sizeof(activate)))
    {
        cerr << "Network: " << __func__ << " failed to read player action flags." << endl;
        return false;
    }

    out_activate = activate;
    return true;
}

//bool ClientMessage::DecodeChangeRegion(sf::TcpSocket& socket, uint16_t& out_region_id)
//{
//    uint16_t region_id;
//
//    if (!read(socket, &region_id, sizeof(region_id)))
//    {
//        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
//        return false;
//    }
//
//    out_region_id = region_id;
//    return true;
//}

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
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
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
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
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
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
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
        buffer_len += sizeof(player.properties);
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
        std::memcpy(buffer + offset, &player.properties, sizeof(player.properties));
        offset += sizeof(player.properties);
    }

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::ChangePlayerProperty(sf::TcpSocket& socket, uint16_t player_id, PlayerProperties properties)
{
    Code code = Code::ChangePlayerProperty;

    constexpr size_t buffer_len = sizeof(code) + sizeof(player_id) + sizeof(properties);
    uint8_t buffer[buffer_len];

    int offset = 0;

    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &properties, sizeof(properties));
    offset += sizeof(properties);

    if (!writeBuffer(socket, buffer, buffer_len))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::OwnerLeft(sf::TcpSocket& socket)
{
    Code code = Code::OwnerLeft;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::StartGame(sf::TcpSocket& socket)
{
    Code code = Code::StartGame;

    if (!writeBuffer(socket, &code, sizeof(code)))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
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
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::SetZone(sf::TcpSocket& socket, definitions::Zone zone)
{
    Code code = Code::SetZone;

    uint16_t num_regions = zone.regions.size();
    uint16_t num_links = zone.links.size();

    size_t buffer_size = sizeof(code);
    buffer_size += sizeof(num_regions) + (sizeof(definitions::Zone::RegionNode) * zone.regions.size());
    buffer_size += sizeof(num_links) + (sizeof(definitions::Zone::Link) * zone.links.size());

    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);

    std::memcpy(buffer + offset, &num_regions, sizeof(num_regions));
    offset += sizeof(num_regions);
    for (auto& region : zone.regions)
    {
        std::memcpy(buffer + offset, &region, sizeof(region));
        offset += sizeof(region);
    }

    std::memcpy(buffer + offset, &num_links, sizeof(num_links));
    offset += sizeof(num_links);
    for (auto& link : zone.links)
    {
        std::memcpy(buffer + offset, &link, sizeof(link));
        offset += sizeof(link);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::SetGuiPause(sf::TcpSocket& socket, bool paused, GuiType gui_type)
{
    Code code = Code::SetGuiPause;
    constexpr size_t buffer_size = sizeof(code) + sizeof(paused) + sizeof(gui_type);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &paused, sizeof(paused));
    offset += sizeof(paused);
    std::memcpy(buffer + offset, &gui_type, sizeof(gui_type));
    offset += sizeof(gui_type);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::PlayerStates(sf::TcpSocket& socket, std::vector<PlayerData> players)
{
    Code code = Code::PlayerStates;
    uint8_t num_players = static_cast<uint8_t>(players.size());

    size_t buffer_size = sizeof(code) + sizeof(num_players) + ((sizeof(uint16_t) + sizeof(float) * 2 + sizeof(PlayerData::health)) * players.size());
    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &num_players, sizeof(num_players));
    offset += sizeof(num_players);

    for (auto& player : players)
    {
        std::memcpy(buffer + offset, &player.id, sizeof(player.id));
        offset += sizeof(player.id);
        std::memcpy(buffer + offset, &player.position.x, sizeof(player.position.x));
        offset += sizeof(player.position.x);
        std::memcpy(buffer + offset, &player.position.y, sizeof(player.position.y));
        offset += sizeof(player.position.y);
        std::memcpy(buffer + offset, &player.health, sizeof(player.health));
        offset += sizeof(player.health);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::PlayerStartAction(sf::TcpSocket& socket, uint16_t player_id, PlayerAction action)
{
    Code code = ServerMessage::Code::PlayerStartAction;

    size_t buffer_size = sizeof(code) + sizeof(player_id) + sizeof(action.flags);
    if (action.flags.start_attack)
    {
        buffer_size += sizeof(action.attack_angle);
    }

    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &action.flags, sizeof(action.flags));
    offset += sizeof(action.flags);

    if (action.flags.start_attack)
    {
        std::memcpy(buffer + offset, &action.attack_angle, sizeof(action.attack_angle));
        offset += sizeof(action.attack_angle);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::EnemyChangeAction(sf::TcpSocket& socket, uint16_t enemy_id, EnemyAnimation action)
{
    Code code = ServerMessage::Code::EnemyChangeAction;

    constexpr size_t buffer_size = sizeof(code) + sizeof(enemy_id) + sizeof(action);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &enemy_id, sizeof(enemy_id));
    offset += sizeof(enemy_id);
    std::memcpy(buffer + offset, &action, sizeof(action));
    offset += sizeof(action);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::ChangeItem(sf::TcpSocket& socket, definitions::ItemType item)
{
    Code code = ServerMessage::Code::ChangeItem;

    constexpr size_t buffer_size = sizeof(code) + sizeof(item);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &item, sizeof(item));
    offset += sizeof(code);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::EnemyUpdate(sf::TcpSocket& socket, std::vector<EnemyData> enemies)
{
    Code code = ServerMessage::Code::EnemyUpdate;

    uint16_t num_enemies = static_cast<uint16_t>(enemies.size());

    size_t enemy_data_size = sizeof(EnemyData::id) + sizeof(EnemyData::position.x) + sizeof(EnemyData::position.y);
    enemy_data_size += sizeof(EnemyData::health) + sizeof(EnemyData::charge);
    size_t buffer_size = sizeof(code) + sizeof(num_enemies) + (enemy_data_size * num_enemies);

    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &num_enemies, sizeof(num_enemies));
    offset += sizeof(num_enemies);

    for (auto& enemy : enemies)
    {
        std::memcpy(buffer + offset, &enemy.id, sizeof(enemy.id));
        offset += sizeof(enemy.id);
        std::memcpy(buffer + offset, &enemy.position.x, sizeof(enemy.position.x));
        offset += sizeof(enemy.position.x);
        std::memcpy(buffer + offset, &enemy.position.y, sizeof(enemy.position.y));
        offset += sizeof(enemy.position.y);
        std::memcpy(buffer + offset, &enemy.health, sizeof(enemy.health));
        offset += sizeof(enemy.health);
        std::memcpy(buffer + offset, &enemy.charge, sizeof(enemy.charge));
        offset += sizeof(enemy.charge);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::BatteryUpdate(sf::TcpSocket& socket, float battery_level)
{
    Code code = ServerMessage::Code::BatteryUpdate;

    constexpr size_t buffer_size = sizeof(code) + sizeof(battery_level);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &battery_level, sizeof(battery_level));
    offset += sizeof(battery_level);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::ProjectileUpdate(sf::TcpSocket& socket, std::vector<ProjectileData> projectiles)
{
    Code code = ServerMessage::Code::ProjectileUpdate;

    uint16_t num_projectiles = projectiles.size();

    size_t buffer_size = sizeof(code) + sizeof(num_projectiles) + (sizeof(ProjectileData::id) + sizeof(ProjectileData::position)) * projectiles.size();
    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &num_projectiles, sizeof(num_projectiles));
    offset += sizeof(num_projectiles);

    for (auto& projectile : projectiles)
    {
        std::memcpy(buffer + offset, &projectile.id, sizeof(projectile.id));
        offset += sizeof(projectile.id);
        std::memcpy(buffer + offset, &projectile.position.x, sizeof(projectile.position.x));
        offset += sizeof(projectile.position.x);
        std::memcpy(buffer + offset, &projectile.position.y, sizeof(projectile.position.y));
        offset += sizeof(projectile.position.y);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::ChangeRegion(sf::TcpSocket& socket, uint16_t region_id)
{
    Code code = ServerMessage::Code::ChangeRegion;

    constexpr size_t buffer_size = sizeof(code) + sizeof(region_id);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &region_id, sizeof(region_id));
    offset += sizeof(region_id);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::UpdateStash(sf::TcpSocket& socket, std::array<definitions::ItemType, 24> items)
{
    Code code = ServerMessage::Code::UpdateStash;

    constexpr size_t buffer_size = sizeof(code) + sizeof(definitions::ItemType) * 24;
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);

    for (unsigned i = 0; i < 24; ++i)
    {
        std::memcpy(buffer + offset, &items[i], sizeof(items[i]));
        offset += sizeof(items[i]);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::GatherPlayers(sf::TcpSocket& socket, uint16_t player_id, bool start)
{
    Code code = ServerMessage::Code::GatherPlayers;

    constexpr size_t buffer_size = sizeof(code) + sizeof(player_id) + sizeof(start);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &start, sizeof(start));
    offset += sizeof(start);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::CastVote(sf::TcpSocket& socket, uint16_t player_id, uint8_t vote, bool confirm)
{
    Code code = ServerMessage::Code::CastVote;

    constexpr size_t buffer_size = sizeof(code) + sizeof(player_id) + sizeof(vote) + sizeof(confirm);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &player_id, sizeof(player_id));
    offset += sizeof(player_id);
    std::memcpy(buffer + offset, &vote, sizeof(vote));
    offset += sizeof(vote);
    std::memcpy(buffer + offset, &confirm, sizeof(confirm));
    offset += sizeof(confirm);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::SetMenuEvent(sf::TcpSocket& socket, uint16_t event_id)
{
    Code code = ServerMessage::Code::SetMenuEvent;

    constexpr size_t buffer_size = sizeof(code) + sizeof(event_id);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &event_id, sizeof(event_id));
    offset += sizeof(event_id);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::AdvanceMenuEvent(sf::TcpSocket& socket, uint16_t page_id, bool finish)
{
    Code code = ServerMessage::Code::AdvanceMenuEvent;

    constexpr size_t buffer_size = sizeof(code) + sizeof(page_id) + sizeof(finish);
    uint8_t buffer[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &page_id, sizeof(page_id));
    offset += sizeof(page_id);
    std::memcpy(buffer + offset, &finish, sizeof(finish));
    offset += sizeof(finish);

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
        return false;
    }

    return true;
}

bool ServerMessage::DisplayPath(sf::TcpSocket& socket, util::PathingGraph graph, std::list<sf::Vector2f> path)
{
    Code code = ServerMessage::Code::DisplayPath;

    uint16_t num_graph_nodes = graph.nodes.size();
    uint16_t num_path_nodes = path.size();

    size_t buffer_size = sizeof(code) + sizeof(num_graph_nodes) + (sizeof(sf::Vector2f) * num_graph_nodes) + sizeof(num_path_nodes) + (sizeof(sf::Vector2f) * num_path_nodes);
    uint8_t* buffer = new uint8_t[buffer_size];

    int offset = 0;
    std::memcpy(buffer, &code, sizeof(code));
    offset += sizeof(code);

    std::memcpy(buffer + offset, &num_graph_nodes, sizeof(num_graph_nodes));
    offset += sizeof(num_graph_nodes);
    for (unsigned i = 0; i < num_graph_nodes; ++i)
    {
        sf::Vector2f node = graph.nodes[i].position;
        std::memcpy(buffer + offset, &node, sizeof(node));
        offset += sizeof(node);
    }

    std::memcpy(buffer + offset, &num_path_nodes, sizeof(num_path_nodes));
    offset += sizeof(num_path_nodes);
    for (auto& node : path)
    {
        std::memcpy(buffer + offset, &node, sizeof(node));
        offset += sizeof(node);
    }

    if (!writeBuffer(socket, buffer, buffer_size))
    {
        cerr << "Network: Failed to send ServerMessage::" << __func__ << " message" << endl;
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
        cerr << "Network: " << __func__ << " failed to read a player id." << endl;
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
        cerr << "Network: " << __func__ << " failed to read a player id." << endl;
        return false;
    }

    if (!readString(socket, name))
    {
        cerr << "Network: " << __func__ << " failed to read a player name." << endl;
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
        cerr << "Network: " << __func__ << " failed to read a player id." << endl;
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
        cerr << "Network: " << __func__ << " failed to read a player id." << endl;
        return false;
    }

    if (!read(socket, &num_players, sizeof(num_players)))
    {
        cerr << "Network: " << __func__ << " failed to read a player count." << endl;
        return false;
    }

    for (int i = 0; i < num_players; ++i)
    {
        PlayerData data{};

        if (!read(socket, &data.id, sizeof(data.id)))
        {
            cerr << "Network: " << __func__ << " failed to read a player id." << endl;
            return false;
        }

        if (!readString(socket, data.name))
        {
            cerr << "Network: " << __func__ << " failed to read a player name." << endl;
            return false;
        }

        if (!read(socket, &data.properties, sizeof(data.properties)))
        {
            cerr << "Network: " << __func__ << " failed to read a player id." << endl;
            return false;
        }

        players.push_back(data);
    }

    out_players = players;
    out_id = player_id;

    return true;
}

bool ServerMessage::DecodeChangePlayerProperty(sf::TcpSocket& socket, uint16_t& out_player_id, PlayerProperties& out_properties)
{
    uint16_t player_id;
    PlayerProperties properties;

    if (!read(socket, &player_id, sizeof(player_id)))
    {
        cerr << "Network: " << __func__ << " failed to read a player id." << endl;
        return false;
    }

    if (!read(socket, &properties, sizeof(properties)))
    {
        cerr << "Network: " << __func__ << " failed to read player properties." << endl;
        return false;
    }

    out_player_id = player_id;
    out_properties = properties;
    return true;
}

bool ServerMessage::DecodeAllPlayersLoaded(sf::TcpSocket& socket, sf::Vector2f& out_spawn_position)
{
    float x;
    float y;

    if (!read(socket, &x, sizeof(x)))
    {
        cerr << "Network: " << __func__ << " failed to read a spawn x position." << endl;
        return false;
    }

    if (!read(socket, &y, sizeof(y)))
    {
        cerr << "Network: " << __func__ << " failed to read a spawn y position." << endl;
        return false;
    }

    out_spawn_position.x = x;
    out_spawn_position.y = y;

    return true;
}

bool ServerMessage::DecodeSetZone(sf::TcpSocket& socket, definitions::Zone& out_zone)
{
    definitions::Zone zone;
    uint16_t regions_size;
    uint16_t links_size;

    if (!read(socket, &regions_size, sizeof(regions_size)))
    {
        cerr << "Network: " << __func__ << " failed to read a region size." << endl;
        return false;
    }

    for (uint16_t i = 0; i < regions_size; ++i)
    {
        definitions::Zone::RegionNode region;

        if (!read(socket, &region, sizeof(region)))
        {
            cerr << "Network: " << __func__ << " failed to read a region." << endl;
            return false;
        }

        zone.regions.push_back(region);
    }

    if (!read(socket, &links_size, sizeof(links_size)))
    {
        cerr << "Network: " << __func__ << " failed to read a links size." << endl;
        return false;
    }

    for (uint16_t i = 0; i < links_size; ++i)
    {
        definitions::Zone::Link link;

        if (!read(socket, &link, sizeof(link)))
        {
            cerr << "Network: " << __func__ << " failed to read a link." << endl;
            return false;
        }

        zone.links.push_back(link);
    }

    out_zone = zone;
    return true;
}

bool ServerMessage::DecodeSetGuiPause(sf::TcpSocket& socket, bool& out_paused, GuiType& out_gui_type)
{
    bool paused;
    GuiType gui_type;

    if (!read(socket, &paused, sizeof(paused)))
    {
        cerr << "Network: " << __func__ << " failed to read paused value." << endl;
        return false;
    }

    if (!read(socket, &gui_type, sizeof(gui_type)))
    {
        cerr << "Network: " << __func__ << " failed to read gui type." << endl;
        return false;
    }

    out_paused = paused;
    out_gui_type = gui_type;
    return true;
}

bool ServerMessage::DecodePlayerStates(sf::TcpSocket& socket, std::vector<PlayerData>& out_players)
{
    std::vector<PlayerData> players;
    uint8_t num_players;

    if (!read(socket, &num_players, sizeof(num_players)))
    {
        cerr << "Network: " << __func__ << " failed to read num players." << endl;
        return false;
    }

    for (size_t i = 0; i < num_players; ++i)
    {
        PlayerData data;
        if (!read(socket, &data.id, sizeof(data.id)))
        {
            cerr << "Network: " << __func__ << " failed to read a player id." << endl;
            return false;
        }

        if (!read(socket, &data.position.x, sizeof(data.position.x)))
        {
            cerr << "Network: " << __func__ << " failed to read an x position." << endl;
            return false;
        }

        if (!read(socket, &data.position.y, sizeof(data.position.y)))
        {
            cerr << "Network: " << __func__ << " failed to read a y position." << endl;
            return false;
        }

        if (!read(socket, &data.health, sizeof(data.health)))
        {
            cerr << "Network: " << __func__ << " failed to read a health value." << endl;
            return false;
        }

        players.push_back(data);
    }

    out_players = players;

    return true;
}

bool ServerMessage::DecodePlayerStartAction(sf::TcpSocket& socket, uint16_t& out_player_id, PlayerAction& out_action)
{
    uint16_t id;
    PlayerAction temp_action;

    if (!read(socket, &id, sizeof(id)))
    {
        cerr << "Network: " << __func__ << " failed to read player id." << endl;
        return false;
    }

    if (!read(socket, &temp_action.flags, sizeof(temp_action.flags)))
    {
        cerr << "Network: " << __func__ << " failed to read player action flags." << endl;
        return false;
    }

    if (temp_action.flags.start_attack)
    {
        if (!read(socket, &temp_action.attack_angle, sizeof(temp_action.attack_angle)))
        {
            cerr << "Network: " << __func__ << " failed to read an attack angle." << endl;
            return false;
        }
    }

    out_player_id = id;
    out_action = temp_action;

    return true;
}

bool ServerMessage::DecodeEnemyChangeAction(sf::TcpSocket& socket, uint16_t& out_enemy_id, EnemyAnimation& out_action)
{
    uint16_t id;
    EnemyAnimation action;

    if (!read(socket, &id, sizeof(id)))
    {
        cerr << "Network: " << __func__ << " failed to read an enemy id." << endl;
        return false;
    }

    if (!read(socket, &action, sizeof(action)))
    {
        cerr << "Network: " << __func__ << " failed to read player action flags." << endl;
        return false;
    }

    out_enemy_id = id;
    out_action = action;

    return true;
}

bool ServerMessage::DecodeChangeItem(sf::TcpSocket& socket, definitions::ItemType& out_item)
{
    definitions::ItemType item;

    if (!read(socket, &item, sizeof(item)))
    {
        cerr << "Network: " << __func__ << " failed to read num enemies." << endl;
        return false;
    }

    out_item = item;
    return true;
}

bool ServerMessage::DecodeEnemyUpdate(sf::TcpSocket& socket, std::vector<EnemyData>& out_enemies)
{
    uint16_t num_enemies;
    std::vector<EnemyData> enemies;

    if (!read(socket, &num_enemies, sizeof(num_enemies)))
    {
        cerr << "Network: " << __func__ << " failed to read num enemies." << endl;
        return false;
    }

    for (int i = 0; i < num_enemies; ++i)
    {
        EnemyData data;

        if (!read(socket, &data.id, sizeof(data.id)))
        {
            cerr << "Network: " << __func__ << " failed to read an enemy id." << endl;
            return false;
        }

        if (!read(socket, &data.position.x, sizeof(data.position.x)))
        {
            cerr << "Network: " << __func__ << " failed to read an enemy position.x." << endl;
            return false;
        }

        if (!read(socket, &data.position.y, sizeof(data.position.y)))
        {
            cerr << "Network: " << __func__ << " failed to read an enemy position.y." << endl;
            return false;
        }

        if (!read(socket, &data.health, sizeof(data.health)))
        {
            cerr << "Network: " << __func__ << " failed to read an enemy health." << endl;
            return false;
        }

        if (!read(socket, &data.charge, sizeof(data.charge)))
        {
            cerr << "Network: " << __func__ << " failed to read an enemy charge." << endl;
            return false;
        }

        enemies.push_back(data);
    }

    out_enemies = enemies;
    return true;
}

bool ServerMessage::DecodeBatteryUpdate(sf::TcpSocket& socket, float& out_battery_level)
{
    float battery_level;

    if (!read(socket, &battery_level, sizeof(battery_level)))
    {
        cerr << "Network: " << __func__ << " failed to read battery level value." << endl;
        return false;
    }

    out_battery_level = battery_level;
    return true;
}

bool ServerMessage::DecodeProjectileUpdate(sf::TcpSocket& socket, std::vector<ProjectileData>& out_projectiles)
{
    uint16_t num_projectiles;
    std::vector<ProjectileData> projectiles;

    if (!read(socket, &num_projectiles, sizeof(num_projectiles)))
    {
        cerr << "Network: " << __func__ << " failed to read num projectiles." << endl;
        return false;
    }

    for (int i = 0; i < num_projectiles; ++i)
    {
        ProjectileData projectile;

        if (!read(socket, &projectile.id, sizeof(projectile.id)))
        {
            cerr << "Network: " << __func__ << " failed to read projectile id." << endl;
            return false;
        }

        if (!read(socket, &projectile.position.x, sizeof(projectile.position.x)))
        {
            cerr << "Network: " << __func__ << " failed to read projectile x position." << endl;
            return false;
        }

        if (!read(socket, &projectile.position.y, sizeof(projectile.position.y)))
        {
            cerr << "Network: " << __func__ << " failed to read projectile y position." << endl;
            return false;
        }

        projectiles.push_back(projectile);
    }

    out_projectiles = projectiles;
    return true;
}

bool ServerMessage::DecodeChangeRegion(sf::TcpSocket& socket, uint16_t& out_region_id)
{
    uint16_t region_id;

    if (!read(socket, &region_id, sizeof(region_id)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    out_region_id = region_id;
    return true;
}

bool ServerMessage::DecodeUpdateStash(sf::TcpSocket& socket, std::array<definitions::ItemType, 24>& out_items)
{
    std::array<definitions::ItemType, 24> items;

    for (unsigned i = 0; i < 24; ++i)
    {
        definitions::ItemType item;
        if (!read(socket, &item, sizeof(item)))
        {
            cerr << "Network: " << __func__ << " failed to read a region name." << endl;
            return false;
        }
        items[i] = item;
    }

    out_items = items;
    return true;
}

bool ServerMessage::DecodeGatherPlayers(sf::TcpSocket& socket, uint16_t& out_player_id, bool& out_start)
{
    uint16_t player_id;
    bool start;

    if (!read(socket, &player_id, sizeof(player_id)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    if (!read(socket, &start, sizeof(start)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    out_player_id = player_id;
    out_start = start;
    return true;
}

bool ServerMessage::DecodeCastVote(sf::TcpSocket& socket, uint16_t& out_player_id, uint8_t& out_vote, bool& out_confirm)
{
    uint16_t player_id;
    uint8_t vote;
    bool confirm;

    if (!read(socket, &player_id, sizeof(player_id)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    if (!read(socket, &vote, sizeof(vote)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    if (!read(socket, &confirm, sizeof(confirm)))
    {
        cerr << "Network: " << __func__ << " failed to read a region id." << endl;
        return false;
    }

    out_player_id = player_id;
    out_vote = vote;
    out_confirm = confirm;
    return true;
}

bool ServerMessage::DecodeSetMenuEvent(sf::TcpSocket& socket, uint16_t& out_event_id)
{
    uint16_t event_id;

    if (!read(socket, &event_id, sizeof(event_id)))
    {
        cerr << "Network: " << __func__ << " failed to read an event id." << endl;
        return false;
    }

    out_event_id = event_id;
    return true;
}

bool ServerMessage::DecodeAdvanceMenuEvent(sf::TcpSocket& socket, uint16_t& out_page_id, bool& out_finish)
{
    uint16_t page_id;
    bool finish;

    if (!read(socket, &page_id, sizeof(page_id)))
    {
        cerr << "Network: " << __func__ << " failed to read an event id." << endl;
        return false;
    }

    if (!read(socket, &finish, sizeof(finish)))
    {
        cerr << "Network: " << __func__ << " failed to read an event id." << endl;
        return false;
    }

    out_page_id = page_id;
    out_finish = finish;
    return true;
}

bool ServerMessage::DecodeDisplayPath(sf::TcpSocket& socket, std::vector<sf::Vector2f>& out_graph, std::vector<sf::Vector2f>& out_path)
{
    std::vector<sf::Vector2f> graph;
    std::vector<sf::Vector2f> path;

    uint16_t graph_size;
    uint16_t path_size;

    if (!read(socket, &graph_size, sizeof(graph_size)))
    {
        cerr << "Network: " << __func__ << " failed to read a graph size value." << endl;
        return false;
    }

    for (unsigned i = 0; i < graph_size; ++i)
    {
        sf::Vector2f node;
        if (!read(socket, &node, sizeof(node)))
        {
            cerr << "Network: " << __func__ << " failed to read a graph node." << endl;
            return false;
        }

        graph.push_back(node);
    }

    if (!read(socket, &path_size, sizeof(path_size)))
    {
        cerr << "Network: " << __func__ << " failed to read a path size value." << endl;
        return false;
    }

    for (unsigned i = 0; i < path_size; ++i)
    {
        sf::Vector2f node;
        if (!read(socket, &node, sizeof(node)))
        {
            cerr << "Network: " << __func__ << " failed to read a graph node." << endl;
            return false;
        }

        path.push_back(node);
    }

    out_graph = graph;
    out_path = path;

    return true;
}

} // server
