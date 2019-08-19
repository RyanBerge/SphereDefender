#pragma once
#include <SFML/Network/TcpSocket.hpp>
#include <string>

namespace Network
{
    enum class MessageType : uint8_t
    {
        Error = 0,
        SetName = 1,
        NotifyChangeName = SetName
    };

    extern const uint32_t ServerPort;

    bool Read(sf::TcpSocket& socket, void* buf, int num_bytes);
    std::string ReadString(sf::TcpSocket& socket);

    bool Write(sf::TcpSocket& socket, const void* buf, int num_bytes);
    bool WriteString(sf::TcpSocket& socket, const std::string& str);
}
