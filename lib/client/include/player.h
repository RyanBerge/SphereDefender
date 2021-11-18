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
#include "player_state.h"

namespace client {

class Player
{
public:
    Player();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(PlayerState local);
    void Unload();

    void SetPosition(sf::Vector2f position);
    sf::Vector2f GetPosition();

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnTextEntered(sf::Event::TextEvent event);

    uint16_t PlayerId;

private:
    sf::CircleShape sphere;
    sf::Vector2f current_destination;
    sf::RectangleShape path;
    double movement_speed = 200; // pixels per second
    std::string name;

};

} // client
