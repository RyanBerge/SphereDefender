/**************************************************************************************************
 *  File:       player_info.h
 *  Class:      PlayerInfo
 *
 *  Purpose:    The representation of players from the server's point-of-view
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <memory>
#include "entity_data.h"
#include "game_math.h"
#include "region_definitions.h"

namespace server {

class PlayerInfo
{
public:
    enum class PlayerStatus
    {
        Uninitialized,
        Disconnected,
        Menus,
        Loading,
        Alive,
        Dead
    };

    struct WeaponKnockback
    {
        float distance; // total distance over duration in pixels
        float duration; // duration in seconds
    };

    void Update(sf::Time elapsed, std::vector<sf::FloatRect> obstacles, definitions::ConvoyDefinition convoy);
    void UpdatePlayerState(sf::Vector2i movement_vector);
    void StartAttack(uint16_t attack_angle);
    sf::FloatRect GetBounds();
    util::LineSegment GetSwordLocation();
    sf::Vector2f GetAttackVector();
    uint8_t GetWeaponDamage();
    WeaponKnockback GetWeaponKnockback();
    void Damage(int damage_value);

    std::shared_ptr<sf::TcpSocket> Socket;
    PlayerStatus Status;
    network::PlayerData Data;

    bool Attacking = false;

private:
    sf::FloatRect getBoundingBox(sf::Vector2f position);

    double movement_speed = definitions::PlayerDefinition::PLAYER_SPEED; // pixels per second
    int swing_speed = 360; // degrees per second
    int swing_arc = 90; // degrees
    int sword_offset = definitions::PlayerDefinition::SWORD_OFFSET;
    int sword_length = definitions::PlayerDefinition::SWORD_LENGTH;

    sf::Vector2f velocity;
    double starting_attack_angle;
    double current_attack_angle;

};

} // server
