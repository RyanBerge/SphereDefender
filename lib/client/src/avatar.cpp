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

using std::cout, std::endl;

namespace client {

namespace {
    const int SWORD_OFFSET = -35;
    const int SWORD_LENGTH = 40;
}

Avatar::Avatar() { }

Avatar::Avatar(sf::Color color, std::string name) : Name{name}
{
    sphere.setRadius(35);
    sphere.setPosition(300, 300);
    sphere.setFillColor(color);
    sphere.setOutlineColor(sf::Color::Black);
    sphere.setOutlineThickness(2);
    sphere.setOrigin(sphere.getLocalBounds().width / 2, sphere.getLocalBounds().height / 2);
    sword.setSize(sf::Vector2f(SWORD_LENGTH, 8));
    sword.setOrigin(sf::Vector2f(SWORD_OFFSET, 4));
    sword.setFillColor(sf::Color::Red);
    sword.setPosition(sphere.getPosition());
}

void Avatar::Update(sf::Time elapsed)
{
    if (Attacking)
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
}

void Avatar::Draw()
{
    if (Attacking)
    {
        GameManager::GetInstance().Window.draw(sword);
    }

    GameManager::GetInstance().Window.draw(sphere);
}

void Avatar::SetPosition(sf::Vector2f position)
{
    sphere.setPosition(position);
}

sf::Vector2f Avatar::GetPosition()
{
    return sphere.getPosition();
}

void Avatar::StartAttack(uint16_t attack_angle)
{
    starting_attack_angle = attack_angle;
    sword.setRotation(starting_attack_angle);
    Attacking = true;
}

} // client
