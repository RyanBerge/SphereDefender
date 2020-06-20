#include "server.h"
#include "network.h"
#include <iostream>
#include <cstring>

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
                owner = player.id;
                cout << "Server initialized by " << player.name << endl;
                player.status = PlayerData::Status::Menus;
            }
            else
            {
                // TODO: Shut down server
            }
        }
        break;
        case Network::ClientMessage::JoinServer:
        {
            if (setName(player))
            {
                cout << player.name << " joined the server" << endl;
                player.status = PlayerData::Status::Menus;
                notifyAllPlayerJoined(player);
            }
        }
        break;
        case Network::ClientMessage::LeaveGame:
        {
            cout << "Player leaving lobby: " << player.name << endl;
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
            code = Network::ServerMessage::LobbyOwner;
            uint8_t owner_message_buffer[sizeof(code) + sizeof(p.id)];

            if (owner < 0)
            {
                // TODO: Respond with an Error code ServerNotInitialized or something
                cerr << "Player attempted to join a lobby with no owner" << endl;
                return;
            }

            uint16_t owner_id = static_cast<uint16_t>(owner);

            std::memcpy(owner_message_buffer, &code, sizeof(code));
            std::memcpy(owner_message_buffer + sizeof(code), &owner_id, sizeof(owner_id));

            if (!Network::Write(*player.socket, owner_message_buffer, sizeof(code) + sizeof(owner_id)))
            {
                cerr << "Something went wrong when notifying " << player.name << " of the owner id for the game" << endl;
            }
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
