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
    util::Milliseconds FLASH_TIMER = 100;
    util::Seconds DESPAWN_TIMER = 3;
}

Enemy::Enemy()
{
    spritesheet.LoadAnimationData("entities/bat.json");
    spritesheet.CenterOrigin();
    spritesheet.SetAnimation("Move");
    spritesheet.SetDebugAnimationPrint(true);
}

void Enemy::Update(sf::Time elapsed)
{
    if (GameManager::GetInstance().Game.IsPaused)
    {
        return;
    }

    damage_timer += elapsed.asSeconds();
    despawn_timer += elapsed.asSeconds();

    spritesheet.Update(elapsed);
    if (damage_flash && damage_timer * 1000 >= FLASH_TIMER)
    {
        damage_flash = false;
        spritesheet.GetSprite().setColor(sf::Color::White);
    }

    if (!alive && despawn_timer > DESPAWN_TIMER)
    {
        Despawn = true;
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
        damage_timer = 0;
        spritesheet.GetSprite().setColor(sf::Color{255, 150, 0});
    }

    data = new_data;
    spritesheet.SetPosition(data.position.x, data.position.y);
    spritesheet.GetSprite().setScale(sf::Vector2f{1 + data.charge / 100, 1 + data.charge / 100});

    if (new_data.id == 1)
    {
        spritesheet.SetAnimation("Sniff");
    }
}

void Enemy::ChangeAction(network::EnemyAction action)
{
    if (!alive)
    {
        return;
    }

    if (action.flags.move)
    {
        spritesheet.SetAnimation("Move");
    }
    else if (action.flags.sniffing)
    {
        spritesheet.SetAnimation("Sniff");
    }
    else if (action.flags.feed)
    {
        spritesheet.SetAnimation("Feed");
    }
    else if (action.flags.leaping)
    {
        spritesheet.SetAnimation("Leap");
    }
    else if (action.flags.knockback)
    {
        spritesheet.SetAnimation("Stunned");
    }
    else if (action.flags.stunned)
    {
        spritesheet.SetAnimation("Stunned");
    }
    else if (action.flags.start_attack)
    {

    }
    else if (action.flags.dead)
    {
        alive = false;
        spritesheet.SetAnimation("Death");
        despawn_timer = 0;
    }
}

network::EnemyData Enemy::GetData()
{
    return data;
}

} // namespace client
