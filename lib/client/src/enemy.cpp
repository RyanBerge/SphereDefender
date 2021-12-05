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
#include <iostream>

using std::cout, std::endl;

namespace client
{

namespace {
    int FLASH_TIMER = 100; // milliseconds
}

Enemy::Enemy()
{
    spritesheet.LoadTexture("SmallDemon.png");
    spritesheet.GetSprite().setOrigin(spritesheet.GetSprite().getLocalBounds().width / 2, spritesheet.GetSprite().getLocalBounds().height / 2);
}

void Enemy::Update(sf::Time elapsed)
{
    (void)elapsed;
    if (damage_flash && damage_timer.getElapsedTime().asMilliseconds() >= FLASH_TIMER)
    {
        damage_flash = false;
        spritesheet.GetSprite().setColor(sf::Color::White);
    }
}

void Enemy::Draw()
{
    spritesheet.Draw();
}

void Enemy::UpdateData(network::EnemyData new_data)
{
    if (new_data.health < data.health)
    {
        damage_flash = true;
        damage_timer.restart();
        spritesheet.GetSprite().setColor(sf::Color{255, 150, 0});
    }

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
