/**************************************************************************************************
 *  File:       region.h
 *  Class:      Region
 *
 *  Purpose:    A single instanced region at a node
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "enemy.h"
#include "player_info.h"
#include <list>
#include "SFML/System/Time.hpp"

namespace server
{

class Region
{
public:
    Region();

    void Update(sf::Time elapsed, std::vector<PlayerInfo>& players);
    void Cull();

    network::ConvoyData Convoy;
    std::list<Enemy> Enemies;

private:

};

} // namespace server
