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

#include "entity_data.h"
#include "spritesheet.h"

namespace client
{

class Enemy
{
public:
    Enemy();

    void Draw();

    network::EnemyData GetData();
    void UpdateData(network::EnemyData new_data);

private:
    Spritesheet spritesheet;
    network::EnemyData data;

};

} // namespace client
