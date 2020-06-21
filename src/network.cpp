#include "network.h"
#include <iostream>
#include <cstring>

using std::cerr, std::endl;

const uint32_t Network::ServerPort = 49879;

bool Network::Read(sf::TcpSocket& socket, void* buf, int num_bytes)
{
    uint8_t* buffer = reinterpret_cast<uint8_t*>(buf);

    int bytes_read = 0;
    while (bytes_read < num_bytes)
    {
        std::size_t b_read;
        auto status = socket.receive(&buffer[bytes_read], num_bytes - bytes_read, b_read);

        if(status == sf::Socket::Error)
        {
            cerr << "ERROR: Read from socket failed before all data has been retrieved..." << endl;
            cerr << "Bytes Read: " << bytes_read << endl;
            return false;
        }
        else if(status == sf::Socket::Disconnected)
        {
            cerr << "Closed socket detected." << endl;
            return false;
        }

        bytes_read += b_read;
        buffer += bytes_read;
    }

    return true;
}

std::string Network::ReadString(sf::TcpSocket& socket)
{
    socket.setBlocking(true);
    uint16_t string_size;

    if (!Read(socket, &string_size, 2))
    {
        cerr << "Socket died while reading a string size." << endl;
        return "";
    }

    char name_buffer[string_size];
    if (!Read(socket, name_buffer, string_size))
    {
        cerr << "Socket died while reading a string." << endl;
        return "";
    }

    socket.setBlocking(false);
    return std::string(name_buffer, string_size);
}

bool Network::Write(sf::TcpSocket& socket, const void* data, int num_bytes)
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

bool Network::WriteString(sf::TcpSocket& socket, const std::string& str)
{
    if (str.size() >= UINT16_MAX)
    {
        cerr << "Write String request over UINT16_MAX" << endl;
    }

    uint16_t size = str.size();

    std::cout << "Writing string: " << str << std::endl;

    if (!Write(socket, &size, 2))
    {
        cerr << "Socket died while writing a string." << endl;
        return false;
    }

    if (!Write(socket, str.c_str(), str.size()))
    {
        cerr << "Socket died while writing a string." << endl;
        return false;
    }
    return true;
}

int Network::CopyStringToBuffer(uint8_t* buffer, int offset, std::string data)
{
    const char* raw_data = data.c_str();
    uint16_t data_size = data.size();
    std::memcpy(buffer + offset, &data_size, sizeof(data_size));
    offset += 2;
    std::memcpy(buffer + offset, raw_data, data_size);

    return data_size + 2;
}
