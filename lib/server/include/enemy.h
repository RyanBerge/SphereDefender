/**************************************************************************************************
 *  File:       enemy.h
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "SFML/System/Vector2.hpp"
#include "SFML/System/Time.hpp"
#include <cstdint>
#include "entity_data.h"
#include "player_info.h"

namespace server {

class Enemy
{
public:
    Enemy();

    void Update(sf::Time elapsed, std::vector<PlayerInfo>& players);

    sf::Vector2f Bounds{};
    network::EnemyData Data;

private:
    double movement_speed = 170;
};

} // server
