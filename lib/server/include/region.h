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

#include "game_math.h"
#include "new_enemy.h"
#include <list>
#include <random>
#include "SFML/System/Clock.hpp"

namespace server
{

class Region
{
public:
    Region();
    Region(definitions::RegionType region_name, int player_count, float battery_level);

    void Update(sf::Time elapsed);
    bool AdvanceMenuEvent(uint16_t winner, uint16_t& out_event_id, uint16_t& out_event_action);

    sf::FloatRect Bounds;
    definitions::ConvoyDefinition Convoy{};
    std::list<Enemy> Enemies;
    std::vector<sf::FloatRect> Obstacles;
    std::list<definitions::Projectile> Projectiles;
    float BatteryLevel = 0;
    bool Leyline = false;

    std::map<definitions::EntityType, util::PathingGraph> PathingGraphs;

private:
    definitions::RegionDefinition definition;
    float region_difficulty = 0;
    int num_players = 1;
    util::Seconds region_age = 0; // In seconds
    util::Seconds age_timer = 0;
    float battery_charge_rate = 0; // Units-per-second
    definitions::MenuEvent current_event;

    void updateBattery(sf::Time elapsed);
    void spawnEnemy(definitions::EntityType type, sf::Vector2f position);
    void spawnEnemy(definitions::EntityType type, sf::Vector2f position, sf::Vector2f pack_position);
    void spawnPack(definitions::EnemyPack pack);
    bool spawnWave(sf::Time elapsed);
    void handleProjectiles(sf::Time elapsed);
};

} // namespace server
