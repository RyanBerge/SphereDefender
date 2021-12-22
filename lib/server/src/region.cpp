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
    constexpr int BASE_SPAWN_INTERVAL = 4;
    constexpr float SPAWN_ACCELERATION_PER_PLAYER = 0.15;
    constexpr float MINIMUM_SPAWN_INTERVAL = 0.8;
}

namespace server {

Region::Region() { }

Region::Region(definitions::RegionName region_name, unsigned player_count, float battery_level) : BatteryLevel{battery_level}, num_players{player_count}
{
    definitions::RegionDefinition definition = definitions::GetRegionDefinition(region_name);

    Convoy = definition.convoy;

    for (auto& obstacle : definition.obstacles)
    {
        Obstacles.push_back(obstacle.bounds);
    }

    spawn_enemies = definition.leyline;

    if (definition.leyline)
    {
        battery_charge_rate = 5;
    }

    spawn_interval = BASE_SPAWN_INTERVAL;

    last_spawn = 0;
}

void Region::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    float region_age = age_timer.getElapsedTime().asSeconds();

    for (auto& enemy : Enemies)
    {
        enemy.Update(elapsed, Convoy, players, Obstacles);
    }

    int siphon_rate = 0;
    for (auto& enemy : Enemies)
    {
        if (enemy.GetBehavior() == Enemy::Behavior::Feeding)
        {
            siphon_rate += enemy.GetSiphonRate();
        }
    }

    if (region_age < 6)
    {
        BatteryLevel -= siphon_rate * elapsed.asSeconds();
    }
    else
    {
        BatteryLevel += (battery_charge_rate - siphon_rate) * elapsed.asSeconds();
        if (spawn_enemies && (last_spawn == 0 || age_timer.getElapsedTime().asSeconds() >= last_spawn + spawn_interval))
        {
            spawnEnemy();
        }
    }

    if (BatteryLevel < 0)
    {
        BatteryLevel = 0;
    }

    if (BatteryLevel > 1000)
    {
        BatteryLevel = 1000;
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
    last_spawn = age_timer.getElapsedTime().asSeconds();
    spawn_interval -= SPAWN_ACCELERATION_PER_PLAYER * num_players;
    if (spawn_interval < MINIMUM_SPAWN_INTERVAL)
    {
        spawn_interval = MINIMUM_SPAWN_INTERVAL;
    }
}

} // namespace server
