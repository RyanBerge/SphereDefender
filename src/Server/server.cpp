#include "server.h"
#include "network.h"
#include <iostream>
#include <cstring>

#include "global_resources.h"

using std::cout, std::cerr, std::endl;

Server::Server()
{
    listener.setBlocking(false);
    listener.listen(Network::ServerPort);
}

bool Server::Update()
{
    // Any new connections?
    listen();

    if (owner != -1 && players.size() == 0)
    {
        cout << "Shutting down" << endl;
        return false;
    }

    // Process any incoming messages
    for (auto iter = players.begin(); iter != players.end(); ++iter)
    {
        auto& player = *iter;
        if (player.status != PlayerData::Status::Disconnected)
        {
            checkMessages(player);
        }
        else
        {
            players.erase(iter--);
        }
    }

    return true;
}

void Server::listen()
{
    static uint16_t player_id = 0;

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

    PlayerData player;
    player.socket = player_socket;
    player.socket->setBlocking(false);
    player.name = "";
    player.id = player_id++;
    player.status = PlayerData::Status::Uninitialized;

    Network::ServerMessage code = Network::ServerMessage::PlayerId;
    size_t buffer_len = sizeof(code) + sizeof(player.id);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + 1, &player.id, sizeof(player.id));

    if (!Network::Write(*player.socket, buffer, buffer_len))
    {
        cerr << "Something went wrong when new player connected." << endl;
        return;
    }

    cout << "New player connected to server with id: " << player.id << endl;

    players.push_back(player);
}

void Server::checkMessages(PlayerData& player)
{
    Network::ClientMessage code;
    size_t bytes_received;

    player.socket->setBlocking(false);
    auto status = player.socket->receive(&code, 1, bytes_received);
    if (status == sf::Socket::Status::NotReady)
    {
        return;
    }
    else if (status != sf::Socket::Status::Done)
    {
        if (status == sf::Socket::Status::Disconnected)
        {
            cerr << "Player disconnected unexpectedly: " << player.name << endl;
            player.status = PlayerData::Status::Disconnected;
            if (player.id != owner)
            {
                notifyAllPlayerLeft(player);
            }
            else
            {
                notifyAllOwnerLeft();
            }

            return;
        }

        cerr << "checkMessages threw a socket error" << endl;
        return;
    }

    switch (code)
    {
        case Network::ClientMessage::Error:
        {
            cerr << "Error codes not yet implemented" << endl;
        }
        break;
        case Network::ClientMessage::InitServer:
        {
            if (setName(player))
            {
                if (game_state != GameState::Uninitialized)
                {
                    cerr << "Server already initialized" << endl;
                    return;
                }

                owner = player.id;
                cout << "Server initialized by " << player.name << endl;
                player.status = PlayerData::Status::Menus;
                game_state = GameState::Lobby;
            }
            else
            {
                cerr << "Something went horribly wrong" << endl;
                // TODO: Shut down server
            }
        }
        break;
        case Network::ClientMessage::JoinServer:
        {
            if (setName(player))
            {
                if (game_state != GameState::Lobby)
                {
                    // TODO: Send error message
                    cout << player.name << " tried to join a game already in progress" << endl;
                    player.socket->disconnect();
                    player.status = PlayerData::Status::Disconnected;
                    return;
                }

                cout << player.name << " joined the server" << endl;
                player.status = PlayerData::Status::Menus;
                notifyAllPlayerJoined(player);
            }
        }
        break;
        case Network::ClientMessage::StartGame:
        {
            if (player.id == owner && game_state == GameState::Lobby)
            {
                cout << player.name << " started a new game!" << endl;
                startGame();
                game_state = GameState::Loading;
            }
            else
            {
                if (player.id != owner)
                {
                    cerr << player.name << " tried to start the game without being the owner" << endl;
                }
                else
                {
                    cerr << "The server was not in the lobby when the owner tried to start the game" << endl;
                }
            }
        }
        break;
        case Network::ClientMessage::LoadingComplete:
        {
            cout << player.name << " has finished loading." << endl;

            if (game_state == GameState::Loading)
            {
                player.status = PlayerData::Status::Alive;

                for (auto& p : players)
                {
                    if (p.status != PlayerData::Status::Alive)
                    {
                        return;
                    }
                }

                cout << "All players have loaded. Notifying." << endl;

                notifyAllPlayersLoaded();
            }
        }
        break;
        case Network::ClientMessage::LeaveGame:
        {
            cout << "Player left the server: " << player.name << endl;
            player.socket->disconnect();
            player.status = PlayerData::Status::Disconnected;
            if (player.id != owner)
            {
                notifyAllPlayerLeft(player);
            }
            else
            {
                notifyAllOwnerLeft();
            }
        }
        break;
        default:
        {
            cerr << "Message code not recognized: " << static_cast<uint8_t>(code) << endl;
        }
    }
}

bool Server::setName(PlayerData& player)
{
    std::string name = Network::ReadString(*player.socket);
    if (name == "")
    {
        cerr << "Something went wrong when receiving a player name." << endl;
        return false;
    }

    player.name = name;
    return true;
}

void Server::notifyAllPlayerJoined(PlayerData& p)
{
    Network::ServerMessage code = Network::ServerMessage::PlayerJoined;

    uint16_t str_size = p.name.size();
    size_t buffer_len = sizeof(code) + sizeof(p.id) + sizeof(str_size) + p.name.size();
    uint8_t buffer[buffer_len];

    size_t offset = 0;
    std::memcpy(buffer + offset, &code, sizeof(code));
    offset += sizeof(code);
    std::memcpy(buffer + offset, &p.id, sizeof(p.id));
    offset += sizeof(p.id);
    std::memcpy(buffer + offset, &str_size, sizeof(str_size));
    offset += sizeof(str_size);
    std::memcpy(buffer + offset, p.name.c_str(), p.name.size());

    for (auto& player : players)
    {
        if (player.id != p.id)
        {
            if (!Network::Write(*player.socket, buffer, buffer_len))
            {
                cerr << "Something went wrong when notifying " << player.name << " of " << p.name << " joining" << endl;
            }
        }
        else
        {
            listPlayersInLobby(p);
        }
    }
}

void Server::notifyAllPlayerLeft(PlayerData& p)
{
    Network::ServerMessage code = Network::ServerMessage::PlayerLeft;

    // TODO: Add a reason field?
    // TODO: Rejoining?

    // code + id
    size_t buffer_len = sizeof(code) + sizeof(p.id);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    std::memcpy(buffer + 1, &p.id, sizeof(p.id));

    for (auto& player : players)
    {
        if (player.id != p.id)
        {
            if (!Network::Write(*player.socket, buffer, buffer_len))
            {
                cerr << "Something went wrong when notifying " << player.name << " of " << p.name << " disconnecting" << endl;
            }
        }
    }
}

void Server::notifyAllOwnerLeft()
{
    Network::ServerMessage code = Network::ServerMessage::OwnerLeft;

    // TODO: In-game or not? Do we care? Rejoining?

    // code
    size_t buffer_len = sizeof(code);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));

    for (auto& player : players)
    {
        if (player.id != owner)
        {
            if (!Network::Write(*player.socket, buffer, buffer_len))
            {
                cerr << "Something went wrong when notifying players that the owner (" << player.name << ") left disconnected" << endl;
                continue;
            }

            player.socket->disconnect();
            player.status = PlayerData::Status::Disconnected;
        }
    }
}

void Server::listPlayersInLobby(PlayerData& p)
{
    Network::ServerMessage code = Network::ServerMessage::PlayersInLobby;

    PlayerData owner_data;
    uint8_t buffer_len = 2;

    for (auto& player : players)
    {
        if (player.id == owner)
        {
            owner_data = player;
        }

        if (player.id != p.id)
        {
            buffer_len += sizeof(player.id);
            buffer_len += 2;
            buffer_len += player.name.size();
        }
        // TODO: Add any future properties here!
    }

    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));
    int offset = 1;
    uint8_t num_players = players.size() - 1;
    std::memcpy(buffer + offset, &num_players, 1);
    offset += 1;

    std::memcpy(buffer + offset, &owner_data.id, sizeof(owner_data.id));
    offset += sizeof(owner_data.id);
    offset += Network::CopyStringToBuffer(buffer, offset, owner_data.name);

    for (auto& player : players)
    {
        if (player.id != owner && player.id != p.id)
        {
            std::memcpy(buffer + offset, &player.id, sizeof(player.id));
            offset += sizeof(player.id);
            offset += Network::CopyStringToBuffer(buffer, offset, player.name);
        }
    }

    if (!Network::Write(*p.socket, buffer, buffer_len))
    {
        cerr << "Something went wrong when notifying " << p.name << " of the owner id for the game" << endl;
    }
}

void Server::startGame()
{
    Network::ServerMessage code = Network::ServerMessage::StartGame;

    size_t buffer_len = sizeof(code);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));

    for (auto& player : players)
    {
        if (player.id != owner)
        {
            if (!Network::Write(*player.socket, buffer, buffer_len))
            {
                cerr << "Something went wrong when notifying players that the game was started" << endl;
                continue;
            }
        }
    }
}

void Server::notifyAllPlayersLoaded()
{
    Network::ServerMessage code = Network::ServerMessage::AllPlayersLoaded;

    size_t buffer_len = sizeof(code);
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, sizeof(code));

    for (auto& player : players)
    {
        if (!Network::Write(*player.socket, buffer, buffer_len))
        {
            cerr << "Something went wrong when notifying players that the game was started" << endl;
            continue;
        }
    }
}
