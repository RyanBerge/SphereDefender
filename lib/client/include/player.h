/**************************************************************************************************
 *  File:       player.h
 *  Class:      Player
 *
 *  Purpose:    The local player
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace client {

class Player
{
public:
    Player();

    void Update(sf::Time elapsed);
    void Draw();

    void Load();
    void Unload();

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnTextEntered(sf::Event::TextEvent event);

private:
    sf::CircleShape sphere;
    sf::Vector2f current_destination;
    sf::RectangleShape path;
    double movement_speed = 200; // pixels per second

};

} // client
