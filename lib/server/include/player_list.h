/**************************************************************************************************
 *  File:       player_list.h
 *
 *  Purpose:    A globally-accessible list of players in the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include "player.h"

namespace server
{
    extern std::vector<Player> PlayerList;
} // namespace server
