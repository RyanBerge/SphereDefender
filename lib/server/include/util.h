/**************************************************************************************************
 *  File:       util.h
 *
 *  Purpose:    Utility functions for the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "new_enemy.h"
#include "player.h"

namespace server
{
    Enemy& GetEnemyById(uint16_t id, std::list<Enemy>& enemies);
    Player& GetPlayerById(uint16_t id, std::vector<Player>& players);

} // namespace server
