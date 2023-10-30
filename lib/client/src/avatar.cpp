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
    spritesheet.LoadAnimationData("entities/player.json");
    spritesheet.CenterOrigin();
    spritesheet.SetAnimation("Idle", "South");
    spritesheet.SetDebugAnimationPrint(true);

    definitions::PlayerDefinition player_definition = definitions::PlayerDefinition::Get();
    sphere.setRadius(player_definition.radius);
    sphere.setFillColor(color);
    sphere.setOutlineColor(sf::Color::Black);
    sphere.setOutlineThickness(1);
    sphere.setOrigin(sphere.getLocalBounds().width / 2, sphere.getLocalBounds().height / 2);

    definitions::Weapon weapon = definitions::GetWeapon(definitions::WeaponType::Sword);

    sword.setSize(sf::Vector2f(weapon.length, 4));
    sword.setOrigin(sf::Vector2f(weapon.offset, 2));
    sword.setFillColor(sf::Color::Red);
    sword.setPosition(sphere.getPosition());

    weapon = definitions::GetWeapon(definitions::WeaponType::BurstGun);

    gun.setSize(sf::Vector2f(weapon.length, 4));
    gun.setOrigin(sf::Vector2f(weapon.offset, 2));
    gun.setFillColor(sf::Color::Black);
    gun.setPosition(sphere.getPosition());

    gunshot.LoadAnimationData("player/gunfire.json");
    gunshot.GetSprite().setOrigin(sf::Vector2f(weapon.offset - weapon.length, gunshot.GetSprite().getGlobalBounds().height / 2));
    gunshot.SetPosition(sphere.getPosition().x, sphere.getPosition().y);
}

void Avatar::Update(sf::Time elapsed)
{
    if (GameManager::GetInstance().Game.IsPaused)
    {
        return;
    }

    attack_timer += elapsed.asSeconds();

    spritesheet.Update(elapsed);

    if (Data.health > 0)
    {
        if (Attacking)
        {
            switch (Data.properties.weapon_type)
            {
                case definitions::WeaponType::Sword:
                {
                    sword.setPosition(sphere.getPosition());
                    sword.rotate(360 * elapsed.asSeconds());

                    float rotation_delta = sword.getRotation() - starting_attack_angle;
                    if (rotation_delta < 0)
                    {
                        rotation_delta += 360;
                    }

                    if (rotation_delta > 90)
                    {
                        Attacking = false;
                    }
                }
                break;
                case definitions::WeaponType::HitscanGun:
                case definitions::WeaponType::BurstGun:
                {
                    gun.setPosition(sphere.getPosition());
                    gunshot.SetPosition(sphere.getPosition());
                    if (attack_timer * 1000 > GUN_TIMER)
                    {
                        Attacking = false;
                    }
                }
                break;
            }
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
                    resources::GetWindow().draw(sword);
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

        //resources::GetWindow().draw(sphere);
        spritesheet.Draw();
    }
}

void Avatar::SetPosition(sf::Vector2f position)
{
    if (position.x - Data.position.x == 0 && position.y - Data.position.y > 0)
    {
        spritesheet.SetAnimation("Idle", "South");
    }
    else if (position.x - Data.position.x == 0 && position.y - Data.position.y < 0)
    {
        spritesheet.SetAnimation("Idle", "North");
    }
    else if (position.x - Data.position.x < 0 && position.y - Data.position.y == 0)
    {
        spritesheet.SetAnimation("Idle", "West");
    }
    else if (position.x - Data.position.x > 0 && position.y - Data.position.y == 0)
    {
        spritesheet.SetAnimation("Idle", "East");
    }
    else if (position.x - Data.position.x > 0 && position.y - Data.position.y > 0)
    {
        spritesheet.SetAnimation("Idle", "Southeast");
    }
    else if (position.x - Data.position.x > 0 && position.y - Data.position.y < 0)
    {
        spritesheet.SetAnimation("Idle", "Northeast");
    }
    else if (position.x - Data.position.x < 0 && position.y - Data.position.y > 0)
    {
        spritesheet.SetAnimation("Idle", "Southwest");
    }
    else if (position.x - Data.position.x < 0 && position.y - Data.position.y < 0)
    {
        spritesheet.SetAnimation("Idle", "Northwest");
    }

    Data.position = position;
    sphere.setPosition(position);
    spritesheet.SetPosition(position);
}

sf::Vector2f Avatar::GetPosition()
{
    return Data.position;
}

sf::FloatRect Avatar::GetGlobalBounds()
{
    return sphere.getGlobalBounds();
}

void Avatar::StartAttack(uint16_t attack_angle)
{
    starting_attack_angle = attack_angle;
    sword.setRotation(starting_attack_angle);
    gun.setRotation(starting_attack_angle);
    gunshot.GetSprite().setRotation(starting_attack_angle);
    attack_timer = 0;
    Attacking = true;
}

void Avatar::UpdateHealth(uint8_t health)
{
    // TODO: Play animation if you take damage?
    Data.health = health;
}

} // client
