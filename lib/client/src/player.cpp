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
#include "game_math.h"
#include "resources.h"
#include <iostream>
#include <cmath>
#include <complex>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Player::Player() : avatar(sf::Color(115, 180, 115), "") { }

void Player::Update(sf::Time elapsed)
{
    avatar.Update(elapsed);
}

void Player::Draw()
{
    avatar.Draw();
}

void Player::Load(network::PlayerData data)
{
    avatar.Name = data.name;
    PlayerId = data.id;
}

void Player::Unload()
{
}

void Player::SetPosition(sf::Vector2f position)
{
    avatar.SetPosition(position);
}

sf::Vector2f Player::GetPosition()
{
    return avatar.GetPosition();
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

        if (!avatar.Attacking)
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
    sf::Vector2f distance_to_destination = GameManager::GetInstance().Window.mapPixelToCoords(point, GameManager::GetInstance().Game.WorldView) - avatar.GetPosition();
    float rotation = std::atan(distance_to_destination.y / distance_to_destination.x) * 180 / util::pi;
    rotation += distance_to_destination.x < 0 ? 180 : 0;
    uint16_t attack_angle = (static_cast<uint16_t>(rotation) + 315) % 360;

    network::PlayerAction action;
    action.start_attack = true;
    action.attack_angle = attack_angle;

    ClientMessage::StartAction(ServerSocket, action);
    avatar.StartAttack(attack_angle);
}

} // client
