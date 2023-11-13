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

void Enemy::ChangeAction(network::EnemyAnimation animation, util::Direction direction)
{
    if (!alive)
    {
        return;
    }

    switch (animation)
    {
        case network::EnemyAnimation::Rest:
        {
            spritesheet.SetAnimation("Rest");
        }
        break;
        case network::EnemyAnimation::Move:
        {
            spritesheet.SetAnimation("Move");
        }
        break;
        case network::EnemyAnimation::Feed:
        {
            spritesheet.SetAnimation("Feed");
        }
        break;
        case network::EnemyAnimation::Knockback:
        {
            spritesheet.SetAnimation("Stunned");
        }
        break;
        case network::EnemyAnimation::Stun:
        {
            spritesheet.SetAnimation("Stunned");
        }
        break;
        case network::EnemyAnimation::Tackle:
        {
            spritesheet.SetAnimation("Tackle");
        }
        break;
        case network::EnemyAnimation::Dead:
        {
            alive = false;
            spritesheet.SetAnimation("Death");
            despawn_timer = 0;
        }
        break;
        case network::EnemyAnimation::Sniff:
        {
            spritesheet.SetAnimation("Sniff");
        }
        break;
        case network::EnemyAnimation::LeapWindup:
        {
            spritesheet.SetAnimation("LeapWindup");
        }
        break;
        case network::EnemyAnimation::Leap:
        {
            spritesheet.SetAnimation("Leap");
        }
        break;
        case network::EnemyAnimation::HopWindup:
        {
            spritesheet.SetAnimation("HopWindup");
        }
        break;
        case network::EnemyAnimation::Hop:
        {
            spritesheet.SetAnimation("Hop");
        }
        break;
        case network::EnemyAnimation::TailSwipe:
        {
            spritesheet.SetAnimation("TailSwipe", Spritesheet::GetAnimationVariant(direction));
        }
        break;
    }
}

network::EnemyData Enemy::GetData()
{
    return data;
}

} // namespace client
