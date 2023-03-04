/**************************************************************************************************
 *  File:       global_state.h
 *
 *  Purpose:    Contains any state information needed globally by the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include "player.h"

namespace server::global
{
    extern std::vector<Player> PlayerList;
    extern bool Paused;
    extern bool GatheringPlayers;
    extern bool RegionSelect;
    extern bool MenuEvent;
} // namespace server::global
