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
#include "game_manager.h"
#include <iostream>
#include "game_math.h"

using std::cout, std::endl;
using namespace shared;

namespace client {

namespace {
    const int GUN_TIMER = 100; // milliseconds
}

Avatar::Avatar() { }

Avatar::Avatar(sf::Color color, network::PlayerData data) : Data{data}
{
    sphere.setRadius(PlayerDefinition::PLAYER_RADIUS);
    sphere.setFillColor(color);
    sphere.setOutlineColor(sf::Color::Black);
    sphere.setOutlineThickness(1);
    sphere.setOrigin(sphere.getLocalBounds().width / 2, sphere.getLocalBounds().height / 2);

    sword.setSize(sf::Vector2f(PlayerDefinition::SWORD_LENGTH, 4));
    sword.setOrigin(sf::Vector2f(-PlayerDefinition::SWORD_OFFSET, 2));
    sword.setFillColor(sf::Color::Red);
    sword.setPosition(sphere.getPosition());

    gun.setSize(sf::Vector2f(PlayerDefinition::GUN_LENGTH, 4));
    gun.setOrigin(sf::Vector2f(PlayerDefinition::GUN_OFFSET, 2));
    gun.setFillColor(sf::Color::Black);
    gun.setPosition(sphere.getPosition());

    gunshot.LoadAnimationData("player/gunfire.json");
    gunshot.GetSprite().setOrigin(sf::Vector2f(PlayerDefinition::GUN_OFFSET - PlayerDefinition::GUN_LENGTH, gunshot.GetSprite().getGlobalBounds().height / 2));
    gunshot.SetPosition(sphere.getPosition().x, sphere.getPosition().y);
}

void Avatar::Update(sf::Time elapsed)
{
    if (Data.health > 0)
    {
        if (Attacking)
        {
            switch (Data.properties.player_class)
            {
                case network::PlayerClass::Melee:
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
                case network::PlayerClass::Ranged:
                {
                    gun.setPosition(sphere.getPosition());
                    gunshot.SetPosition(sphere.getPosition());
                    if (attack_timer.getElapsedTime().asMilliseconds() > GUN_TIMER)
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
                    GameManager::GetInstance().Window.draw(sword);
                }
                break;
                case network::PlayerClass::Ranged:
                {
                    gunshot.Draw();
                    GameManager::GetInstance().Window.draw(gun);
                }
                break;
            }
        }

        GameManager::GetInstance().Window.draw(sphere);
    }
}

void Avatar::SetPosition(sf::Vector2f position)
{
    Data.position = position;
    sphere.setPosition(position);
}

sf::Vector2f Avatar::GetPosition()
{
    return Data.position;
}

void Avatar::StartAttack(uint16_t attack_angle)
{
    starting_attack_angle = attack_angle;
    sword.setRotation(starting_attack_angle);
    gun.setRotation(starting_attack_angle);
    gunshot.GetSprite().setRotation(starting_attack_angle);
    attack_timer.restart();
    Attacking = true;
}

void Avatar::UpdateHealth(uint8_t health)
{
    // TODO: Play animation if you take damage?
    Data.health = health;
}

} // client
