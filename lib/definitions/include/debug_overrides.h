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

extern DebugOverride<int> PlayerMovementSpeed;

} // debug
