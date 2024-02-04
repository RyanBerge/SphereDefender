/**************************************************************************************************
 *  File:       convoy.h
 *  Class:      Convoy
 *
 *  Purpose:    A convoy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include "definitions.h"
#include "spritesheet.h"

namespace client
{

class Convoy
{
public:
    static constexpr int CONSOLE_INTERACTION_DISTANCE = 75;

    Convoy();
    Convoy(definitions::ConvoyDefinition convoy_definition);

    void Update(sf::Time elapsed);
    void Draw();

    void LeaveRegion();
    void EnterRegion();
    std::vector<sf::FloatRect> GetInteractablePositions();
    void ClearInteractions();

    sf::Vector2f GetPosition();
    sf::Vector2f GetConsolePosition();
    sf::Vector2f GetStashPosition();

private:
    Spritesheet convoy_sprite;
    Spritesheet console;
    definitions::ConvoyDefinition definition;
    sf::Vector2f base_position;

    bool leaving_region = false;
    bool entering_region = false;
};

} // namespace client

