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
#include <random>
#include "SFML/System/Clock.hpp"

namespace server
{

class Region
{
public:
    Region();
    Region(unsigned num_players);

    void Update(sf::Time elapsed, std::vector<PlayerInfo>& players);
    void Cull();

    network::ConvoyData Convoy;
    std::list<Enemy> Enemies;

private:
    unsigned num_players;
    sf::Clock spawn_timer;

    void spawnEnemy();
};

} // namespace server
