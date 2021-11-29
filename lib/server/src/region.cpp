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
#include <algorithm>

namespace server {

Region::Region()
{
    for (float i = 0; i < 3; ++i)
    {
        Enemy enemy;
        enemy.Data.position = sf::Vector2f{500, 200 * i};
        enemies.push_back(enemy);
    }
}

void Region::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    (void)elapsed;
    for (auto& enemy : enemies)
    {
        enemy.Update(elapsed, players);
    }
}

void Region::Cull()
{
    std::list<Enemy>::iterator end = std::remove_if(enemies.begin(), enemies.end(), [](Enemy enemy){ return enemy.Data.health == 0; });
    enemies.erase(end, enemies.end());

    if (enemies.size() == 0)
    {
        for (float i = 0; i < 3; ++i)
        {
            Enemy enemy;
            enemy.Data.position = sf::Vector2f{500, 200 * i};
            enemies.push_back(enemy);
        }
    }
}

} // namespace server
