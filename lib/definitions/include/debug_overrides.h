/**************************************************************************************************
 *  File:       debug_overrides.h
 *
 *  Purpose:    Debug override values for development
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

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
extern DebugOverride<bool> StaticMap;
extern DebugOverride<int> StartingRegion;

void LoadDebugConfig();

} // debug
