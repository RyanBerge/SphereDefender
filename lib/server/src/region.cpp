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
    const int MAX_ENEMIES = 5;
    const float SPAWN_TIMER = 1; // seconds
}

namespace server {

Region::Region() : Region(1) { }


Region::Region(unsigned num_players) : num_players{num_players}
{
    Convoy.orientation = shared::GetRegionDefinition("default").convoy.orientation;
    Convoy.position = shared::GetRegionDefinition("default").convoy.position;

    for (auto& obstacle : shared::GetRegionDefinition("default").obstacles)
    {
        Obstacles.push_back(obstacle.bounds);
    }

    for (unsigned i = 0; i < num_players * 3; ++i)
    {
        spawnEnemy();
    }

    battery_charge_rate = 5;
}

void Region::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    (void)elapsed;
    for (auto& enemy : Enemies)
    {
        enemy.Update(elapsed, Convoy, players, Obstacles);
    }

    if (Enemies.size() < num_players * MAX_ENEMIES)
    {
        if (spawn_timer.getElapsedTime().asSeconds() > SPAWN_TIMER / num_players)
        {
            spawnEnemy();
        }
    }

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

void Region::Cull()
{
    std::list<Enemy>::iterator end = std::remove_if(Enemies.begin(), Enemies.end(), [](Enemy enemy){ return enemy.Despawn; });
    Enemies.erase(end, Enemies.end());
}

void Region::spawnEnemy()
{
    static std::random_device random_device;
    static std::mt19937 random_generator{random_device()};
    static std::uniform_int_distribution<> distribution_x(900, 1100);
    static std::uniform_int_distribution<> distribution_y(-400, 600);

    Enemy enemy;

    enemy.Data.position.x = distribution_x(random_generator);
    enemy.Data.position.y = distribution_y(random_generator);
    //enemy.Data.position = sf::Vector2f{900, 0};
    Enemies.push_back(enemy);
    spawn_timer.restart();
}

} // namespace server
