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
#include "SFML/Graphics/RectangleShape.hpp"
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
    void OnKeyPressed(sf::Event::KeyEvent event);
    void OnKeyReleased(sf::Event::KeyEvent event);

    uint16_t PlayerId;

private:
    sf::Vector2i movement{0, 0};
    sf::Vector2i velocity{0, 0};
    sf::CircleShape sphere;
    sf::RectangleShape sword;
    double movement_speed = 200; // pixels per second
    std::string name;
    bool attacking = false;
    float starting_attack_angle;

    void updateMovement();
    void startAttack(sf::Vector2i point);

};

} // client
