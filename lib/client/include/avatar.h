/**************************************************************************************************
 *  File:       avatar.h
 *  Class:      Avatar
 *
 *  Purpose:    Everything needed to render player avatars and what they are doing
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include "SFML/Graphics/RectangleShape.hpp"
#include "spritesheet.h"
#include "entity_data.h"
#include "game_math.h"

namespace client {

class Avatar {
public:
    Avatar();
    Avatar(sf::Color color, network::PlayerData data);

    void Update(sf::Time elapsed);
    void Draw();

    void SetPosition(sf::Vector2f position);
    sf::Vector2f GetPosition();
    sf::Vector2f GetPathingHitbox();

    void StartAttack(uint16_t attack_angle);
    void UpdateHealth(uint8_t health);

    network::PlayerData Data;
    bool Attacking = false;

private:
    Spritesheet spritesheet;
    sf::RectangleShape gun;
    Spritesheet gunshot;

    float starting_attack_angle;
    util::Seconds attack_timer;

};

} // client
