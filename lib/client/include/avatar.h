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

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include "SFML/Graphics/RectangleShape.hpp"
#include <cstdint>
#include <string>
#include "entity_data.h"

namespace client {

class Avatar {
public:
    Avatar();
    Avatar(sf::Color color, network::PlayerData data);

    void Update(sf::Time elapsed);
    void Draw();

    void SetPosition(sf::Vector2f position);
    sf::Vector2f GetPosition();

    void StartAttack(uint16_t attack_angle);
    void UpdateHealth(uint8_t health);

    network::PlayerData Data;
    bool Attacking = false;

private:
    sf::CircleShape sphere;
    sf::RectangleShape sword;

    float starting_attack_angle;

};

} // client
