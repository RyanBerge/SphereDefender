/**************************************************************************************************
 *  File:       player_state.h
 *
 *  Purpose:    Struct for holding information about other players
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "messaging.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/CircleShape.hpp>

namespace client {

struct PlayerState
{
    network::PlayerData data;
    sf::Text lobby_display_text;
    sf::CircleShape sphere;
};

} // client
