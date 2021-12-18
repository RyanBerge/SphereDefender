/**************************************************************************************************
 *  File:       convoy.cpp
 *  Class:      Convoy
 *
 *  Purpose:    A convoy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "convoy.h"
#include "resources.h"
#include "game_math.h"
#include <iostream>

using std::cout, std::endl;

namespace client {
namespace {
    constexpr int CONVOY_SPEED = 150;
    constexpr int CONVOY_SPAWN_OFFSET = 900;
}

Convoy::Convoy() { }

Convoy::Convoy(definitions::ConvoyDefinition definition)
{
    base_position = definition.position;
    rectangle.setSize(definitions::GetConvoyBounds(definition).getSize());

    rectangle.setOrigin(rectangle.getSize().x / 2, rectangle.getSize().y / 2);
    rectangle.setPosition(definition.position);
    rectangle.setFillColor(sf::Color(115, 150, 180));
    rectangle.setOutlineColor(sf::Color::Black);
    rectangle.setOutlineThickness(2);

    console.LoadAnimationData("entities/console.json");
    console.SetPosition(sf::Vector2f{rectangle.getGlobalBounds().left + rectangle.getGlobalBounds().width - 30, rectangle.getGlobalBounds().top + 150});
}

void Convoy::Update(sf::Time elapsed)
{
    console.Update(elapsed);

    if (leaving_region)
    {
        sf::Vector2f velocity = sf::Vector2f{0, -CONVOY_SPEED} * elapsed.asSeconds();
        rectangle.move(velocity);
        console.SetPosition(console.GetSprite().getPosition() + velocity);
    }
    else if (entering_region)
    {
        sf::Vector2f velocity = sf::Vector2f{0, -CONVOY_SPEED} * elapsed.asSeconds();
        rectangle.move(velocity);
        console.SetPosition(console.GetSprite().getPosition() + velocity);
        if (rectangle.getPosition().y < base_position.y)
        {
            rectangle.setPosition(base_position);
            console.SetPosition(sf::Vector2f{rectangle.getGlobalBounds().left + rectangle.getGlobalBounds().width - 30, rectangle.getGlobalBounds().top + 150});
            entering_region = false;
        }
    }
}

void Convoy::Draw()
{
    resources::GetWindow().draw(rectangle);
    console.Draw();
}

void Convoy::LeaveRegion()
{
    leaving_region = true;
}

void Convoy::EnterRegion()
{
    entering_region = true;
    rectangle.setPosition(rectangle.getPosition().x, rectangle.getPosition().y + CONVOY_SPAWN_OFFSET);
    console.SetPosition(console.GetSprite().getPosition().x, console.GetSprite().getPosition().y + CONVOY_SPAWN_OFFSET);
}

double Convoy::UpdateInteractables(double distance, sf::Vector2f player_position)
{
    double new_distance = util::Distance(console.GetSprite().getPosition(), player_position);
    if (new_distance <= CONSOLE_INTERACTION_DISTANCE && new_distance < distance)
    {
        console.SetAnimation("Focus");
        distance = new_distance;
    }
    else
    {
        console.SetAnimation("Default");
    }

    return new_distance;
}

void Convoy::ClearInteractions()
{
    console.SetAnimation("Default");
}

sf::Vector2f Convoy::GetPosition()
{
    return rectangle.getPosition();
}

sf::Vector2f Convoy::GetConsolePosition()
{
    return console.GetSprite().getPosition();
}

} // client
