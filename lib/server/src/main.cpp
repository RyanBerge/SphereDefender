/**************************************************************************************************
 *  File:       main.cpp
 *  Library:    Server
 *
 *  Purpose:    Main function for the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "server.h"
#include "debug_overrides.h"

using std::cout, std::cerr, std::endl;

void load_debug_config()
{
    std::filesystem::path config_path("../data/configs/debug_overrides.json");
    if (!std::filesystem::exists(config_path))
    {
        cerr << "Could not open debug override config." << endl;
        return;
    }

    std::ifstream file(config_path);
    nlohmann::json json;
    file >> json;

    auto& speed_override = json["player_movement_speed"];
    if (speed_override["override"])
    {
        debug::PlayerMovementSpeed.override = true;
        debug::PlayerMovementSpeed.value = speed_override["value"];
    }
}

int main()
{
    load_debug_config();

    server::Server server;
    server.Start();
}
