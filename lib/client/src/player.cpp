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
#include "settings.h"
#include "game_math.h"
#include "resources.h"
#include "messaging.h"
#include <iostream>
#include <cmath>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

namespace client {
namespace {
    constexpr int GUN_ATTACK_COOLDOWN = 300; // milliseconds
}

Player::Player() : Avatar(sf::Color(115, 180, 115), network::PlayerData{}) { }

void Player::Update(sf::Time elapsed)
{
    attack_timer += elapsed.asSeconds();

    Avatar.Update(elapsed);

    if (Avatar.Data.properties.weapon_type == definitions::WeaponType::BurstGun)
    {
        if (attacking && attack_timer * 1000 >= definitions::GetWeapon(Avatar.Data.properties.weapon_type).attack_cooldown)
        {
            startAttack(sf::Mouse::getPosition(resources::GetWindow()));
            attack_timer = 0;
        }
    }
}

void Player::Draw()
{
    Avatar.Draw();
}

void Player::Load(network::PlayerData data)
{
    Avatar.Data = data;
    Avatar.Data.health = 100;
    Avatar.SetPosition(data.position);
}

void Player::Unload()
{
}

void Player::SetPosition(sf::Vector2f position)
{
    Avatar.SetPosition(position);
}

sf::Vector2f Player::GetPosition()
{
    return Avatar.GetPosition();
}

void Player::SetActionsEnabled(bool enabled)
{
    if (enabled)
    {
        actions_disabled = false;
        updateMovement();
    }
    else
    {
        actions_disabled = true;
        ClientMessage::PlayerStateChange(resources::GetServerSocket(), sf::Vector2i{0, 0});
    }
}

bool Player::ActionsDisabled()
{
    return actions_disabled;
}


void Player::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    (void)event;
}

void Player::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    if (actions_disabled)
    {
        return;
    }

    if (event.button == sf::Mouse::Button::Right)
    {
        sf::Vector2i click_point{event.x, event.y};

        if (attack_timer * 1000 >= definitions::GetWeapon(Avatar.Data.properties.weapon_type).attack_cooldown)
        {
            startAttack(click_point);
        }
    }
}

void Player::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    if (event.button == sf::Mouse::Button::Right && attacking)
    {
        attacking = false;
    }
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
    if (actions_disabled)
    {
        return;
    }

    if (Avatar.Data.health > 0)
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

        ClientMessage::PlayerStateChange(resources::GetServerSocket(), movement_vector);
    }
}

void Player::startAttack(sf::Vector2i point)
{
    if (Avatar.Data.health > 0)
    {
        sf::Vector2f distance_to_destination = resources::GetWindow().mapPixelToCoords(point, resources::GetWorldView()) - Avatar.GetPosition();
        float rotation = std::atan2(distance_to_destination.y, distance_to_destination.x) * 180 / util::pi;

        uint16_t attack_angle = (static_cast<uint16_t>(rotation + 360)) % 360;

        network::PlayerAction action;
        action.flags.start_attack = true;
        action.attack_angle = attack_angle;

        ClientMessage::StartAction(resources::GetServerSocket(), action);
        Avatar.StartAttack(attack_angle);

        attacking = true;
        attack_timer = 0;
    }
}

} // client
