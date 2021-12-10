/**************************************************************************************************
 *  File:       enemy.h
 *  Class:      Enemy
 *
 *  Purpose:    The client representation of an enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "messaging.h"
#include "spritesheet.h"

namespace client
{

class Enemy
{
public:
    Enemy();

    void Update(sf::Time elapsed);
    void Draw();

    network::EnemyData GetData();
    void UpdateData(network::EnemyData new_data);
    void ChangeAction(network::EnemyAction action);

private:
    Spritesheet spritesheet;
    network::EnemyData data;

    bool attacking;
    sf::Vector2f attack_vector;
    sf::Vector2f attack_starting_position;

    bool damage_flash = false;
    sf::Clock damage_timer;
};

} // namespace client
