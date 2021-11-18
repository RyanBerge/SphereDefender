/**************************************************************************************************
 *  File:       player.cpp
 *  Class:      Player
 *
 *  Purpose:    The local player
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "player.h"
#include "game_manager.h"
#include "util.h"
#include <iostream>
#include <cmath>

using std::cout, std::cerr, std::endl;

namespace client {

Player::Player()
{
    sphere.setRadius(35);
    sphere.setPosition(300, 300);
    sphere.setFillColor(sf::Color(115, 180, 115));
    sphere.setOutlineColor(sf::Color::Black);
    sphere.setOutlineThickness(2);
    sphere.setOrigin(sphere.getLocalBounds().width / 2, sphere.getLocalBounds().height / 2);
    current_destination = sphere.getPosition();
    path.setSize(sf::Vector2f(200, 2));
    path.setFillColor(sf::Color::Red);
    path.setPosition(current_destination);
}

void Player::Update(sf::Time elapsed)
{
    sf::Vector2f distance_to_destination = current_destination - sphere.getPosition();
    double length = std::hypot(distance_to_destination.x, distance_to_destination.y);

    if (length < 2)
    {
        sphere.setPosition(current_destination);
    }

    if (sphere.getPosition() != current_destination)
    {
        sphere.move(sf::Vector2f(distance_to_destination.x / length * movement_speed * elapsed.asSeconds(), distance_to_destination.y / length * movement_speed * elapsed.asSeconds()));
    }

    path.setSize(sf::Vector2f(length , 2));
    path.setPosition(sphere.getPosition());
    float rotation = std::atan(distance_to_destination.y / distance_to_destination.x) * 180 / util::pi;
    rotation += distance_to_destination.x < 0 ? 180 : 0;
    path.setRotation(rotation);
}

void Player::Draw()
{
    GameManager::GetInstance().Window.draw(path);
    GameManager::GetInstance().Window.draw(sphere);
}

void Player::Load(PlayerState local)
{
    name = local.data.name;
    PlayerId = local.data.id;
}

void Player::Unload()
{
}

void Player::SetPosition(sf::Vector2f position)
{
    sphere.setPosition(position);
    current_destination = sphere.getPosition();
}

sf::Vector2f Player::GetPosition()
{
    return sphere.getPosition();
}

void Player::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    (void)event;
}

void Player::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    if (event.button == sf::Mouse::Button::Right)
    {
        current_destination = GameManager::GetInstance().Window.mapPixelToCoords(sf::Vector2i(event.x, event.y), GameManager::GetInstance().Game.WorldView);
    }
}

void Player::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    (void)event;
}

void Player::OnTextEntered(sf::Event::TextEvent event)
{
    (void)event;
}

} // client
