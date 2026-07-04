/**************************************************************************************************
 *  File:       player_stats.h
 *  Class:      PlayerStats
 *
 *  Purpose:    A central place to keep track of all mutable player attributes
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "definitions.h"
#include <map>

namespace client::stats {

class PlayerStats
{
public:
    PlayerStats();

    int UnspentSkillPoints = 0;
    int TotalSkillPoints = 0;
    std::map<definitions::SkillType, bool> Skills;
};

PlayerStats& GetPlayerStats();

} // namespace client
