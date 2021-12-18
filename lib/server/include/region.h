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
    Region(definitions::RegionName region_name, unsigned player_count, float battery_level);

    void Update(sf::Time elapsed, std::vector<PlayerInfo>& players);
    void Cull();

    definitions::ConvoyDefinition Convoy{};
    std::list<Enemy> Enemies;
    std::vector<sf::FloatRect> Obstacles;
    float BatteryLevel = 0;

private:
    unsigned num_players = 1;
    sf::Clock spawn_timer;
    float battery_charge_rate = 0; // Units-per-second
    bool spawn_enemies = false;
    bool delaying_enemy_spawn = true;
    bool charging = false;

    void spawnEnemy();
};

} // namespace server
