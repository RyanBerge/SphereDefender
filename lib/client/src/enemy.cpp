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
    spritesheet.GetSprite().setOrigin(spritesheet.GetSprite().getLocalBounds().width / 2, spritesheet.GetSprite().getLocalBounds().height / 2);
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

void Enemy::StartAction(network::EnemyAction action)
{
    (void)action;
}

network::EnemyData Enemy::GetData()
{
    return data;
}

} // namespace client
