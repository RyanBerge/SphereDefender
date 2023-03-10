/**************************************************************************************************
 *  File:       main.cpp
 *  Library:    Server
 *
 *  Purpose:    Main function for the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "server.h"
#include "debug_overrides.h"

int main()
{
    debug::LoadDebugConfig();

    server::Server server;
    server.Start();
}
