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
#include "game_math.h"

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
    void ChangeAction(network::EnemyAnimation action);

    bool Despawn = false;

private:
    Spritesheet spritesheet;
    network::EnemyData data;

    bool alive = true;
    util::Seconds despawn_timer;

    bool attacking;
    sf::Vector2f attack_vector;
    sf::Vector2f attack_starting_position;

    bool damage_flash = false;
    util::Seconds damage_timer;
};

} // namespace client
