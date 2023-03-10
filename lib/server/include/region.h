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

#include "types.h"
#include "enemy.h"
#include <list>
#include <random>
#include "SFML/System/Clock.hpp"

namespace server
{

class Region
{
public:
    Region();
    Region(definitions::RegionType region_name, unsigned player_count, float battery_level);

    void Update(sf::Time elapsed);
    void Cull();
    bool AdvanceMenuEvent(uint16_t winner, uint16_t& out_event_id, uint16_t& out_event_action);

    definitions::ConvoyDefinition Convoy{};
    std::list<Enemy> Enemies;
    std::vector<sf::FloatRect> Obstacles;
    std::list<definitions::Projectile> Projectiles;
    float BatteryLevel = 0;

private:
    unsigned num_players = 1;
    util::Seconds region_age = 0; // In seconds
    float spawn_interval;
    float last_spawn;
    float battery_charge_rate = 0; // Units-per-second
    bool spawn_enemies = false;
    bool charging = false;
    definitions::MenuEvent current_event;

    void spawnEnemy();
    void handleProjectiles(sf::Time elapsed);
};

} // namespace server
