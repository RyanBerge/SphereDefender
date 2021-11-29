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

using network::ConvoyData;

namespace server {

Region::Region()
{
    Convoy.orientation = ConvoyData::Orientation::North;
    Convoy.position = sf::Vector2f(-600, 0);

    for (float i = 0; i < 3; ++i)
    {
        Enemy enemy;
        enemy.Data.position = sf::Vector2f{500, 200 * i};
        Enemies.push_back(enemy);
    }
}

void Region::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    (void)elapsed;
    for (auto& enemy : Enemies)
    {
        enemy.Update(elapsed, players, Convoy);
    }
}

void Region::Cull()
{
    std::list<Enemy>::iterator end = std::remove_if(Enemies.begin(), Enemies.end(), [](Enemy enemy){ return enemy.Data.health == 0; });
    Enemies.erase(end, Enemies.end());

    if (Enemies.size() == 0)
    {
        for (float i = 0; i < 3; ++i)
        {
            Enemy enemy;
            enemy.Data.position = sf::Vector2f{500, 200 * i};
            Enemies.push_back(enemy);
        }
    }
}

} // namespace server
