/**************************************************************************************************
 *  File:       debug_overrides.h
 *
 *  Purpose:    Debug override values for development
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "SFML/System/Vector2.hpp"

namespace debug
{

template <class T>
struct DebugOverride
{
    bool override;
    T value;
};

extern DebugOverride<bool> StartInGame;
extern DebugOverride<int> PlayerMovementSpeed;
extern DebugOverride<sf::Vector2f> PlayerSpawnPoint;
extern DebugOverride<bool> StaticMap;
extern DebugOverride<int> StartingRegion;

void LoadDebugConfig();

} // debug
