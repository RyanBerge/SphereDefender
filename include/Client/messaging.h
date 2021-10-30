#pragma once
#include "network.h"

namespace Network
{
    sf::Socket::Status Connect(std::string ip);
    bool Read(void* buf, size_t num_bytes);
    std::string ReadString();
    bool Write(const void* data, int numBytes);
    bool WriteString(const std::string& str);
}

namespace Message
{
    void CheckMessages();

    void InitializeServer(std::string name);
    void JoinServer(std::string name);
    void StartGame();
    void LoadingComplete();
    void LeaveGame();
}
