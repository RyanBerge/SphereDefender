/**************************************************************************************************
 *  File:       util.cpp
 *
 *  Purpose:    Utility functions for the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "util.h"

namespace server
{

Enemy& GetEnemyById(uint16_t id, std::list<Enemy>& enemies)
{
    for (auto& enemy : enemies)
    {
        if (enemy.Data.id == id)
        {
            return enemy;
        }
    }

    throw std::runtime_error("Enemy Id not found.");
}

Player& GetPlayerById(uint16_t id, std::vector<Player>& players)
{
    for (auto& player : players)
    {
        if (player.Data.id == id)
        {
            return player;
        }
    }

    throw std::runtime_error("Player Id not found.");
}

} // namespace server
