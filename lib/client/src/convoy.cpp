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

Convoy::Convoy(definitions::ConvoyDefinition convoy_definition) : definition{convoy_definition}
{
    convoy_sprite.LoadAnimationData("entities/convoy_vertical.json");
    convoy_sprite.SetShadow(true);
    convoy_sprite.SetPosition(definition.Position);
    convoy_sprite.SetAnimation("Default");
    sf::FloatRect sprite_bounds = convoy_sprite.GetSprite().getGlobalBounds();

    console.LoadAnimationData("entities/console.json");
    console.SetPosition(sf::Vector2f{sprite_bounds.left + 65, sprite_bounds.top + 95});

    base_position = definition.Position;
}

void Convoy::Update(sf::Time elapsed)
{
    convoy_sprite.Update(elapsed);
    console.Update(elapsed);

    if (leaving_region)
    {
        sf::Vector2f velocity = sf::Vector2f{0, -CONVOY_SPEED} * elapsed.asSeconds();
        convoy_sprite.SetPosition(convoy_sprite.GetSprite().getPosition() + velocity);
        console.SetPosition(console.GetSprite().getPosition() + velocity);
    }
    else if (entering_region)
    {
        sf::Vector2f velocity = sf::Vector2f{0, -CONVOY_SPEED} * elapsed.asSeconds();
        convoy_sprite.SetPosition(convoy_sprite.GetSprite().getPosition() + velocity);
        console.SetPosition(console.GetSprite().getPosition() + velocity);
        if (convoy_sprite.GetSprite().getPosition().y < base_position.y)
        {
            convoy_sprite.SetPosition(base_position);
            sf::FloatRect sprite_bounds = convoy_sprite.GetSprite().getGlobalBounds();
            console.SetPosition(sf::Vector2f{sprite_bounds.left + 65, sprite_bounds.top + 70});
            entering_region = false;
        }
    }
}

void Convoy::Draw()
{
    convoy_sprite.Draw();
    console.Draw();
}

void Convoy::LeaveRegion()
{
    leaving_region = true;
}

void Convoy::EnterRegion()
{
    entering_region = true;
    convoy_sprite.SetPosition(convoy_sprite.GetSprite().getPosition().x, convoy_sprite.GetSprite().getPosition().y + CONVOY_SPAWN_OFFSET);
    console.SetPosition(console.GetSprite().getPosition().x, console.GetSprite().getPosition().y + CONVOY_SPAWN_OFFSET);
}

std::vector<sf::FloatRect> Convoy::GetInteractablePositions()
{
    std::vector<sf::FloatRect> bounds_list;
    bounds_list.push_back(console.GetSprite().getGlobalBounds());
    bounds_list.push_back(sf::FloatRect(convoy_sprite.GetSprite().getGlobalBounds().left + 62, convoy_sprite.GetSprite().getGlobalBounds().top + 321, 72, 19));

    return bounds_list;
}

void Convoy::ClearInteractions()
{
    console.SetAnimation("Default");
}

sf::Vector2f Convoy::GetPosition()
{
    return convoy_sprite.GetSprite().getPosition();
}

sf::Vector2f Convoy::GetConsolePosition()
{
    return console.GetSprite().getPosition();
}

sf::Vector2f Convoy::GetStashPosition()
{
    return sf::Vector2f{convoy_sprite.GetSprite().getGlobalBounds().left + 62 + 72 / 2, convoy_sprite.GetSprite().getGlobalBounds().top + 321 + 19 / 2};
}

} // client
