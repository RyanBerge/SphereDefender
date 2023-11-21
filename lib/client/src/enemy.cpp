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

Enemy::Enemy() : Enemy(definitions::EntityType::Bat) { }

Enemy::Enemy(definitions::EntityType type)
{
    std::string filename = definitions::GetEntityDefinition(type).animation_definition_file;

    spritesheet.LoadAnimationData("entities/" + filename);
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
        despawn = true;
    }
}

void Enemy::Draw()
{
    if (!despawn)
    {
        spritesheet.Draw();
    }
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
}

void Enemy::ChangeAnimation(definitions::AnimationName animation_name, util::Direction direction)
{
    if (!alive)
    {
        return;
    }

    if (animation_name == "TailSwipe")
    {
        spritesheet.SetAnimation(animation_name, definitions::GetAnimationVariant(direction));
    }
    else
    {
        spritesheet.SetAnimation(animation_name);
    }

    if (animation_name == "Death")
    {
        alive = false;
        despawn_timer = 0;
    }
}

network::EnemyData Enemy::GetData()
{
    return data;
}

} // namespace client
