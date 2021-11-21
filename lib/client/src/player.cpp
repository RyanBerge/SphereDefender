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
#include "settings.h"
#include "util.h"
#include <iostream>
#include <cmath>
#include <complex>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Player::Player()
{
    sphere.setRadius(35);
    sphere.setPosition(300, 300);
    sphere.setFillColor(sf::Color(115, 180, 115));
    sphere.setOutlineColor(sf::Color::Black);
    sphere.setOutlineThickness(2);
    sphere.setOrigin(sphere.getLocalBounds().width / 2, sphere.getLocalBounds().height / 2);
    sword.setSize(sf::Vector2f(40, 8));
    sword.setOrigin(sf::Vector2f(-35, 1.5));
    sword.setFillColor(sf::Color::Red);
    sword.setPosition(sphere.getPosition());
}

void Player::Update(sf::Time elapsed)
{
    if (attacking)
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
            attacking = false;
        }
    }
}

void Player::Draw()
{
    if (attacking)
    {
        GameManager::GetInstance().Window.draw(sword);
    }

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
        sf::Vector2i click_point{event.x, event.y};
        //sf::Vector2f distance_to_destination = click_point - sphere.getPosition();
        //double length = std::hypot(distance_to_destination.x, distance_to_destination.y);

        if (!attacking)
        {
            startAttack(click_point);
        }
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

void Player::OnKeyPressed(sf::Event::KeyEvent event)
{
    Settings::KeyBindings& bindings = Settings::GetInstance().Bindings;

    if (event.code == bindings.MoveLeft || event.code == bindings.MoveRight ||
        event.code == bindings.MoveUp || event.code == bindings.MoveDown)
    {
        updateMovement();
    }
}

void Player::OnKeyReleased(sf::Event::KeyEvent event)
{
    Settings::KeyBindings& bindings = Settings::GetInstance().Bindings;

    if (event.code == bindings.MoveLeft || event.code == bindings.MoveRight ||
        event.code == bindings.MoveUp || event.code == bindings.MoveDown)
    {
        updateMovement();
    }
}

void Player::updateMovement()
{
    Settings::KeyBindings bindings = Settings::GetInstance().Bindings;

    sf::Vector2i movement_vector{};

    if (sf::Keyboard::isKeyPressed(bindings.MoveLeft))
    {
        --movement_vector.x;
    }

    if (sf::Keyboard::isKeyPressed(bindings.MoveRight))
    {
        ++movement_vector.x;
    }

    if (sf::Keyboard::isKeyPressed(bindings.MoveUp))
    {
        --movement_vector.y;
    }

    if (sf::Keyboard::isKeyPressed(bindings.MoveDown))
    {
        ++movement_vector.y;
    }

    ClientMessage::PlayerStateChange(ServerSocket, movement_vector);
}

void Player::startAttack(sf::Vector2i point)
{
    sf::Vector2f distance_to_destination = GameManager::GetInstance().Window.mapPixelToCoords(point, GameManager::GetInstance().Game.WorldView) - sphere.getPosition();
    float rotation = std::atan(distance_to_destination.y / distance_to_destination.x) * 180 / util::pi;
    rotation += distance_to_destination.x < 0 ? 180 : 0;
    starting_attack_angle = (static_cast<int>(rotation) + 315) % 360;
    sword.setRotation(starting_attack_angle);
    attacking = true;
}

} // client
