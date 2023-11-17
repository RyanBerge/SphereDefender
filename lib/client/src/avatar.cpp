/**************************************************************************************************
 *  File:       avatar.h
 *  Class:      Avatar
 *
 *  Purpose:    Everything needed to render player avatars and what they are doing
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "avatar.h"
#include "resources.h"
#include <iostream>
#include "game_math.h"
#include "game_manager.h"

using std::cout, std::endl;

namespace client {

namespace {
    const int GUN_TIMER = 100; // milliseconds
}

Avatar::Avatar() { }

Avatar::Avatar(sf::Color color, network::PlayerData data) : Data{data}
{
    (void)color;
    spritesheet.LoadAnimationData("entities/player.json");
    spritesheet.SetAnimation("Idle", definitions::AnimationVariant::South);

    definitions::Weapon weapon = definitions::GetWeapon(definitions::WeaponType::Sword);

    weapon = definitions::GetWeapon(definitions::WeaponType::BurstGun);

    gun.setSize(sf::Vector2f(weapon.length, 4));
    gun.setOrigin(sf::Vector2f(weapon.offset, 2));
    gun.setFillColor(sf::Color::Black);

    gunshot.LoadAnimationData("player/gunfire.json");
    gunshot.GetSprite().setOrigin(sf::Vector2f(weapon.offset - weapon.length, gunshot.GetSprite().getGlobalBounds().height / 2));
}

void Avatar::Update(sf::Time elapsed)
{
    if (GameManager::GetInstance().Game.IsPaused)
    {
        return;
    }

    attack_timer += elapsed.asSeconds();
    stun_timer += elapsed.asSeconds();

    spritesheet.Update(elapsed);

    if (Data.health > 0)
    {
        if (Attacking)
        {
            switch (Data.properties.weapon_type)
            {
                case definitions::WeaponType::Sword:
                {
                    if (spritesheet.GetAnimation().name != "SwordAttack")
                    {
                        Attacking = false;
                    }
                }
                break;
                case definitions::WeaponType::HitscanGun:
                case definitions::WeaponType::BurstGun:
                {
                    gun.setPosition(spritesheet.GetSprite().getPosition());
                    gunshot.SetPosition(spritesheet.GetSprite().getPosition());
                    if (attack_timer * 1000 > GUN_TIMER)
                    {
                        Attacking = false;
                    }
                }
                break;
            }
        }

        if (stunned && stun_timer >= stun_duration)
        {
            stunned = false;
        }
    }
}

void Avatar::Draw()
{
    if (Data.health > 0)
    {
        if (Attacking)
        {
            switch (Data.properties.player_class)
            {
                case network::PlayerClass::Melee:
                {
                }
                break;
                case network::PlayerClass::Ranged:
                {
                    gunshot.Draw();
                    resources::GetWindow().draw(gun);
                }
                break;
            }
        }

        spritesheet.Draw();
    }
}

void Avatar::SetPosition(sf::Vector2f position)
{
    sf::Vector2f old_position = Data.position;
    Data.position = position;
    spritesheet.SetPosition(position);

    if (spritesheet.GetAnimation().name != "Idle")
    {
        return;
    }

    if (stunned)
    {
        return;
    }

    if (position.x - old_position.x == 0 && position.y - old_position.y > 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::South);
    }
    else if (position.x - old_position.x == 0 && position.y - old_position.y < 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::North);
    }
    else if (position.x - old_position.x < 0 && position.y - old_position.y == 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::West);
    }
    else if (position.x - old_position.x > 0 && position.y - old_position.y == 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::East);
    }
    else if (position.x - old_position.x > 0 && position.y - old_position.y > 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::Southeast);
    }
    else if (position.x - old_position.x > 0 && position.y - old_position.y < 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::Northeast);
    }
    else if (position.x - old_position.x < 0 && position.y - old_position.y > 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::Southwest);
    }
    else if (position.x - old_position.x < 0 && position.y - old_position.y < 0)
    {
        spritesheet.SetAnimation("Idle", definitions::AnimationVariant::Northwest);
    }
}

sf::Vector2f Avatar::GetPosition()
{
    return Data.position;
}

sf::Vector2f Avatar::GetCollisionDimensions()
{
    return spritesheet.GetCollisionDimensions();
}

void Avatar::StartAttack(uint16_t attack_angle)
{
    starting_attack_angle = attack_angle;
    gun.setRotation(starting_attack_angle);
    gunshot.GetSprite().setRotation(starting_attack_angle);
    attack_timer = 0;
    Attacking = true;

    if (Data.properties.player_class != network::PlayerClass::Melee)
    {
        return;
    }

    spritesheet.SetAnimation("SwordAttack", Spritesheet::GetAnimationVariant(util::GetOctalDirection(attack_angle)));
}

void Avatar::SetStunned(util::Seconds duration)
{
    stunned = true;
    stun_duration = duration + 0.05f;
    stun_timer = 0;
}

void Avatar::UpdateHealth(uint8_t health)
{
    // TODO: Play animation if you take damage?
    Data.health = health;
}

} // client
