#include "messaging.h"
#include <SFML/Network/IpAddress.hpp>
#include <cstring>
#include <iostream>
#include "player.h"
#include "state_manager.h"
#include "game_manager.h"
#include "player_states.h"

using std::cerr, std::endl;

namespace {
    sf::TcpSocket server_socket;
    bool is_initialized = false;

    void setPlayerId();
    void playerJoined();
    void playerLeft();
    void ownerLeft();
    void getPlayerData();
}

sf::Socket::Status Network::Connect(std::string ip)
{
    server_socket.setBlocking(true);
    sf::Socket::Status status = server_socket.connect(sf::IpAddress(ip), ServerPort, sf::seconds(5));

    if (status == sf::Socket::Status::Done)
    {
        is_initialized = true;
    }

    return status;
}

bool Network::Read(void* buf, size_t num_bytes)
{
    return Network::Read(server_socket, buf, num_bytes);
}

std::string Network::ReadString()
{
    return Network::ReadString(server_socket);
}

bool Network::Write(const void* data, int num_bytes)
{
    return Write(server_socket, data, num_bytes);
}

bool Network::WriteString(const std::string& str)
{
    return WriteString(server_socket, str);
}

void Message::CheckMessages()
{
    if (!is_initialized)
    {
        return;
    }

    Network::ServerMessage code;
    size_t bytes_received;

    server_socket.setBlocking(false);
    auto status = server_socket.receive(&code, 1, bytes_received);
    if (status == sf::Socket::Status::NotReady)
    {
        return;
    }
    else if (status != sf::Socket::Status::Done)
    {
        if (status == sf::Socket::Status::Disconnected)
        {
            cerr << "Disconnected from server" << endl;

            // this maybe will need to be changed
            is_initialized = false;

            return;
        }

        cerr << "CheckMessages threw a socket error: " << status << endl;
        return;
    }

    switch (code)
    {
        case Network::ServerMessage::Error:
        {
            cerr << "Error codes not yet implemented" << endl;
        }
        break;
        case Network::ServerMessage::PlayerId:
        {
            setPlayerId();
        }
        break;
        case Network::ServerMessage::PlayerJoined:
        {
            playerJoined();
        }
        break;
        case Network::ServerMessage::PlayerLeft:
        {
            playerLeft();
        }
        break;
        case Network::ServerMessage::OwnerLeft:
        {
            ownerLeft();
        }
        break;
        case Network::ServerMessage::PlayersInLobby:
        {
            getPlayerData();
        }
        break;
        case Network::ServerMessage::StartGame:
        {
            std::cout << "The game was started!" << std::endl;
            StateManager::MainMenu::current_menu = MenuType::LoadingScreen;
            GameManager::GetInstance().LoadGame();
        }
        break;
        case Network::ServerMessage::AllPlayersLoaded:
        {
            std::cout << "All players have loaded!" << std::endl;
            StateManager::MainMenu::current_menu = MenuType::None;
            StateManager::Game::state = GameState::Running;
            StateManager::global_state = GlobalState::Game;
        }
        break;
    }
}

namespace {
    void setPlayerId()
    {
        if (!Network::Read(&Player::state.id, sizeof(Player::state.id)))
        {
            // TODO: error of some sort
            std::cerr << "Failed to read player ID" << std::endl;
            return;
        }

        std::cout << "Received player id: " << Player::state.id << std::endl;
        Players::AddPlayer(Player::state);
    }

    void playerJoined()
    {
        uint16_t player_id;
        if (!Network::Read(&player_id, sizeof(player_id)))
        {
            // TODO: Error
            std::cerr << "Failed to read player ID from joining player" << std::endl;
            return;
        }

        std::string player_name = Network::ReadString();
        if (player_name == "")
        {
            // TODO: Error
            std::cerr << "Failed to read player name from joining player" << std::endl;
            return;
        }

        std::cout << player_name << " joined the game with ID: " << player_id << std::endl;

        PlayerState player_state = {};
        player_state.id = player_id;
        player_state.name = player_name;

        Players::AddPlayer(player_state);
    }

    void playerLeft()
    {
        uint16_t player_id;
        if (!Network::Read(&player_id, sizeof(player_id)))
        {
            // TODO: Error
            std::cerr << "Failed to read player ID from leaving player" << std::endl;
            return;
        }

        std::cout << "Player with ID: " << player_id << " left the game" << std::endl;

        Players::RemovePlayer(player_id);
    }

    void ownerLeft()
    {
        std::cout << "Owner left the lobby." << std::endl;
        server_socket.disconnect();
        is_initialized = false;
        StateManager::MainMenu::current_menu = MenuType::Main;

        Players::Clear();
        // TODO: Actually handle this case
    }

    void getPlayerData()
    {
        uint8_t num_players = 0;
        if (!Network::Read(&num_players, sizeof(num_players)))
        {
            // TODO: Error
            std::cerr << "Failed to read player ID" << std::endl;
            return;
        }

        std::cout << "There are " << (int)num_players << " players currently in the game" << std::endl;

        for (int i = 0; i < num_players; ++i)
        {
            PlayerState state;
            if (!Network::Read(&state.id, sizeof(state.id)))
            {
                // TODO: Error
                std::cerr << "Failed to read player ID" << std::endl;
                return;
            }

            state.name = Network::ReadString(server_socket);
            if (state.name == "")
            {
                std::cerr << "Something went wrong when reading a player name" << std::endl;
                return;
            }

            // TODO: Read any future properties here!

            std::cout << "Received PlayerData for: " << state.name << " (id: " << state.id << ")" << std::endl;

            Players::AddPlayer(state);
            if (i == 0)
            {
                Players::owner = state.id;
                std::cout << "Lobby is owned by: " << state.name << std::endl;
            }
        }
    }
}

// This is called when a client creates a new lobby, immediately after launching the server process
void Message::InitializeServer(std::string name)
{
    Network::ClientMessage code = Network::ClientMessage::InitServer;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = 1 + 2 + str_len;
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, 1);
    std::memcpy(buffer + 1, &str_len, 2);
    std::memcpy(buffer + 3, name.c_str(), str_len);

    if (!Network::Write(buffer, buffer_len))
    {
        cerr << "Network: Something went wrong when initialize the server" << endl;
    }
}

void Message::JoinServer(std::string name)
{
    Network::ClientMessage code = Network::ClientMessage::JoinServer;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = 1 + 2 + str_len;
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, 1);
    std::memcpy(buffer + 1, &str_len, 2);
    std::memcpy(buffer + 3, name.c_str(), str_len);

    if (!Network::Write(buffer, buffer_len))
    {
        cerr << "Network: Something went wrong when trying to set the player name" << endl;
    }
}

void Message::StartGame()
{
    Network::ClientMessage code = Network::ClientMessage::StartGame;
    if (!Network::Write(&code, sizeof(code)))
    {
        cerr << "Network: Something went wrong when trying to send a StartGame message" << endl;
    }
}

void Message::LoadingComplete()
{
    Network::ClientMessage code = Network::ClientMessage::LoadingComplete;
    if (!Network::Write(&code, sizeof(code)))
    {
        cerr << "Network Something went wrong when trying to send a LoadingComplete message" << endl;
    }
}

void Message::LeaveGame()
{
    Network::ClientMessage code = Network::ClientMessage::LeaveGame;

    if (!Network::Write(&code, sizeof(code)))
    {
        cerr << "Network: Something went wrong when trying to notify the server on leaving the lobby" << endl;
    }

    is_initialized = false;
}
