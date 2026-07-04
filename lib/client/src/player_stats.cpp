/**************************************************************************************************
 *  File:       player_stats.h
 *  Class:      PlayerStats
 *
 *  Purpose:    A central place to keep track of all mutable player attributes
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "player_stats.h"


namespace client::stats {
namespace {
}

PlayerStats::PlayerStats()
{
    Skills[definitions::SkillType::DodgeRoll] = false;
    Skills[definitions::SkillType::Lunge] = false;
}

PlayerStats& GetPlayerStats()
{
    static PlayerStats stats;
    return stats;
}

} // namespace client
