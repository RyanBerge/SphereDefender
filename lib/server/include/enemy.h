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
#include "SFML/System/Clock.hpp"
#include <cstdint>
#include <list>
#include "entity_data.h"
#include "player_info.h"
#include "region_definitions.h"

namespace server {

class Enemy
{
public:
    Enemy();

    void Update(sf::Time elapsed, std::vector<PlayerInfo>& players, shared::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles);

    sf::FloatRect GetBounds();
    network::EnemyData Data;

private:
    double movement_speed = 210;
    sf::Vector2f destination;
    std::list<sf::Vector2f> path;
    double attack_range = 35;
    int attack_damage = 30;
    bool attacking = false;
    bool attack_flag = false;
    sf::Clock attack_timer;
    sf::Vector2f attack_vector;
    sf::Vector2f starting_attack_position;
    int attack_duration = 250; // milliseconds
    int attack_cooldown = 500; // milliseconds

    void move(sf::Time elapsed, std::vector<PlayerInfo>& players, shared::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles);
    void checkAttack(std::vector<PlayerInfo>& players, shared::ConvoyDefinition convoy);
};

} // server
