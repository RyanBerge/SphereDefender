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
#include <SFML/Window/Event.hpp>
#include "avatar.h"
#include "messaging.h"

namespace client {

class Player
{
public:
    Player();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(network::PlayerData data);
    void Unload();

    void SetPosition(sf::Vector2f position);
    sf::Vector2f GetPosition();

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnTextEntered(sf::Event::TextEvent event);
    void OnKeyPressed(sf::Event::KeyEvent event);
    void OnKeyReleased(sf::Event::KeyEvent event);

    Avatar Avatar;

private:
    void updateMovement();
    void startAttack(sf::Vector2i point);

};

} // client
