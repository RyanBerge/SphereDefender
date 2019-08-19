#include "messaging.h"
#include <SFML/Network/IpAddress.hpp>
#include <cstring>
#include <iostream>

using std::cerr, std::endl;

namespace {
    sf::TcpSocket server_socket;
}

sf::Socket::Status Network::Connect(std::string ip)
{
    server_socket.setBlocking(true);
    return server_socket.connect(sf::IpAddress(ip), ServerPort, sf::seconds(5));
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


void Message::setName(std::string name)
{
    Network::MessageType code = Network::MessageType::SetName;
    uint16_t str_len = name.size();

    // code + len + string
    size_t buffer_len = 1 + 2 + str_len;
    uint8_t buffer[buffer_len];

    std::memcpy(buffer, &code, 1);
    std::memcpy(buffer + 1, &str_len, 2);
    std::memcpy(buffer + 3, name.c_str(), str_len);

    if (!Network::Write(buffer, buffer_len))
    {
        cerr << "Something went wrong when trying to set the player name" << endl;
    }
}
