/**************************************************************************************************
 *  File:       server_settings.h
 *
 *  Purpose:    Settings used by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <cstdint>

namespace network
{
    struct ServerSettings
    {
        uint32_t ServerPort;
    };
} // namespace network
