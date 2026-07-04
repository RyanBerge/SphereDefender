/**************************************************************************************************
 *  File:       skill_definitions.h
 *
 *  Purpose:    Information about skills
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "game_math.h"
#include "SFML/System/Vector2.hpp"

namespace definitions::skills
{

struct Roll
{
    util::Seconds cooldown;
    util::Seconds duration;
    util::DistanceUnits distance;
};

struct SkillDefinitions
{
    Roll roll;
};

SkillDefinitions& GetSkillDefinitions();

} // namespace definitions
