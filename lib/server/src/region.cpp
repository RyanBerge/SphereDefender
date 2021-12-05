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
    const int MAX_ENEMIES = 3;
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
}

void Region::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    (void)elapsed;
    for (auto& enemy : Enemies)
    {
        enemy.Update(elapsed, players, Convoy, Obstacles);
    }

    if (Enemies.size() < num_players * MAX_ENEMIES)
    {
        if (spawn_timer.getElapsedTime().asSeconds() > SPAWN_TIMER / num_players)
        {
            spawnEnemy();
        }
    }
}

void Region::Cull()
{
    std::list<Enemy>::iterator end = std::remove_if(Enemies.begin(), Enemies.end(), [](Enemy enemy){ return enemy.Data.health == 0; });
    Enemies.erase(end, Enemies.end());
}

void Region::spawnEnemy()
{
    static std::random_device random_device;
    static std::mt19937 random_generator{random_device()};
    static std::uniform_int_distribution<> distribution_x(1000, 1100);
    static std::uniform_int_distribution<> distribution_y(-300, 300);

    Enemy enemy;

    enemy.Data.position.x = distribution_x(random_generator);
    enemy.Data.position.y = distribution_y(random_generator);
    //enemy.Data.position = sf::Vector2f{900, 0};
    Enemies.push_back(enemy);
    spawn_timer.restart();
}

} // namespace server
