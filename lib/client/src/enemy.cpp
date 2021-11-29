/**************************************************************************************************
 *  File:       enemy.cpp
 *  Class:      Enemy
 *
 *  Purpose:    The client representation of an enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "enemy.h"
#include "game_manager.h"

namespace client
{

Enemy::Enemy()
{
    spritesheet.LoadTexture("SmallDemon.png");
}

void Enemy::Draw()
{
    spritesheet.Draw();
}

void Enemy::UpdateData(network::EnemyData new_data)
{
    data = new_data;
    spritesheet.SetPosition(data.position.x, data.position.y);
}

network::EnemyData Enemy::GetData()
{
    return data;
}

} // namespace client
