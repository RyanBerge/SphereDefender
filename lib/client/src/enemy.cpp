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
    spritesheet.LoadAnimationData("entities/small_demon.json");
    spritesheet.CenterOrigin();
    spritesheet.SetAnimation("Move");
}

void Enemy::Update(sf::Time elapsed)
{
    spritesheet.Update(elapsed);
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
    spritesheet.GetSprite().setScale(sf::Vector2f{1 + data.charge / 100, 1 + data.charge / 100});
}

void Enemy::ChangeAction(network::EnemyAction action)
{
    if (action.flags.move)
    {
        spritesheet.SetAnimation("Move");
    }
    else if (action.flags.feed)
    {
        spritesheet.SetAnimation("Feed");
    }
    else if (action.flags.knockback)
    {

    }
    else if (action.flags.start_attack)
    {

    }
}

network::EnemyData Enemy::GetData()
{
    return data;
}

} // namespace client
