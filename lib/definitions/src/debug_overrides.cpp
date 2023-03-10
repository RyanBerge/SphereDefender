/**************************************************************************************************
 *  File:       debug_overrides.h
 *
 *  Purpose:    Debug override values for development
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "debug_overrides.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>

using std::cout, std::cerr, std::endl;

namespace debug
{

DebugOverride<int> PlayerMovementSpeed = { false, 100 };
DebugOverride<bool> StaticMap = { false, false };
DebugOverride<int> StartingRegion = { false, 0 };
DebugOverride<bool> StartInGame = { false, false };

void LoadDebugConfig()
{
#ifndef NDEBUG
    std::filesystem::path config_path("../data/configs/debug_overrides.json");
    if (!std::filesystem::exists(config_path))
    {
        cerr << "Could not open debug override config." << endl;
        return;
    }

    try
    {
        std::ifstream file(config_path);
        nlohmann::json json;
        file >> json;

        auto& start_in_game = json["start_in_game"];
        if (start_in_game["override"])
        {
            debug::StartInGame.override = true;
            debug::StartInGame.value = start_in_game["value"];
        }

        auto& speed_override = json["player_movement_speed"];
        if (speed_override["override"])
        {
            debug::PlayerMovementSpeed.override = true;
            debug::PlayerMovementSpeed.value = speed_override["value"];
        }

        auto& map_override = json["static_map"];
        if (map_override["override"])
        {
            debug::StaticMap.override = true;
            debug::StaticMap.value = map_override["value"];
        }

        auto& start_override = json["starting_region"];
        if (start_override["override"])
        {
            debug::StartingRegion.override = true;
            debug::StartingRegion.value = start_override["value"];
        }
    }
    catch(const std::exception& e)
    {
        cerr << "Failed to parse debug config file." << endl;
    }
#endif
}

} // debug
