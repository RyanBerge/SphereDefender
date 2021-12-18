/**************************************************************************************************
 *  File:       region.cpp
 *  Class:      Region
 *
 *  Purpose:    A single instanced region at a node
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region.h"
#include "region_definitions.h"
#include <algorithm>
#include <iostream>

using std::cout, std::endl;

namespace {
    constexpr int MAX_ENEMIES = 5;
    constexpr float SPAWN_TIMER = 1; // seconds
    constexpr int SPAWN_DELAY = 8; // seconds
}

namespace server {

Region::Region() { }

Region::Region(shared::RegionName region_name, unsigned player_count, float battery_level) : BatteryLevel{battery_level}, num_players{player_count}
{
    shared::RegionDefinition definition = shared::GetRegionDefinition(region_name);

    Convoy.orientation = definition.convoy.orientation;
    Convoy.position = definition.convoy.position;

    for (auto& obstacle : definition.obstacles)
    {
        Obstacles.push_back(obstacle.bounds);
    }

    spawn_enemies = definition.leyline;

    if (definition.leyline)
    {
        battery_charge_rate = 5;
    }
}

void Region::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    for (auto& enemy : Enemies)
    {
        enemy.Update(elapsed, Convoy, players, Obstacles);
    }

    if (spawn_enemies)
    {
        if (delaying_enemy_spawn && spawn_timer.getElapsedTime().asSeconds() > SPAWN_DELAY)
        {
            delaying_enemy_spawn = false;
        }
        else if (!delaying_enemy_spawn)
        {
            if (Enemies.size() < num_players * MAX_ENEMIES)
            {
                if (spawn_timer.getElapsedTime().asSeconds() > SPAWN_TIMER / num_players)
                {
                    spawnEnemy();
                }
            }
        }
    }

    if (!charging && spawn_timer.getElapsedTime().asSeconds() > 6)
    {
        charging = true;
    }

    if (charging)
    {
        int siphon_rate = 0;
        for (auto& enemy : Enemies)
        {
            if (enemy.GetBehavior() == Enemy::Behavior::Feeding)
            {
                siphon_rate += enemy.GetSiphonRate();
            }
        }

        BatteryLevel += (battery_charge_rate - siphon_rate) * elapsed.asSeconds();
        if (BatteryLevel < 0)
        {
            BatteryLevel = 0;
        }

        if (BatteryLevel > 1000)
        {
            BatteryLevel = 1000;
        }
    }
}

void Region::Cull()
{
    Enemies.remove_if([](Enemy& enemy){ return enemy.Despawn; });
}

void Region::spawnEnemy()
{
    static std::random_device random_device;
    static std::mt19937 random_generator{random_device()};
    static std::uniform_int_distribution<> distribution_x(450, 550);
    static std::uniform_int_distribution<> distribution_y(-200, 300);

    Enemy enemy;

    enemy.Data.position.x = distribution_x(random_generator);
    enemy.Data.position.y = distribution_y(random_generator);
    //enemy.Data.position = sf::Vector2f{900, 0};

    Enemies.push_back(enemy);
    spawn_timer.restart();
}

} // namespace server
