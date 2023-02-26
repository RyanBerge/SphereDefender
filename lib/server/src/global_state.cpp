/**************************************************************************************************
 *  File:       global_state.h
 *
 *  Purpose:    Contains any state information needed globally by the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "global_state.h"

namespace server::global
{

std::vector<Player> PlayerList;
bool Paused = false;
bool GatheringPlayers = false;
bool RegionSelect = false;

} // namespace server
