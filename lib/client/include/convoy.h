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
#include "region_definitions.h"
#include "spritesheet.h"

namespace client
{

class Convoy
{
public:
    static constexpr int CONSOLE_INTERACTION_DISTANCE = 75;

    Convoy();
    Convoy(definitions::ConvoyDefinition definition);

    void Update(sf::Time elapsed);
    void Draw();

    void LeaveRegion();
    void EnterRegion();
    double UpdateInteractables(double distance, sf::Vector2f player_position);
    void ClearInteractions();

    sf::Vector2f GetPosition();
    sf::Vector2f GetConsolePosition();

private:
    sf::RectangleShape rectangle;
    Spritesheet console;
    sf::Vector2f base_position;

    bool leaving_region = false;
    bool entering_region = false;
};

} // namespace client

