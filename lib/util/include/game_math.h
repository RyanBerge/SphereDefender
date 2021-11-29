/**************************************************************************************************
 *  File:       game_math.h
 *
 *  Purpose:    Miscellaneous math functions
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Rect.hpp"
#include <cmath>

namespace util
{
    constexpr double pi = 3.141592653589793238462643383279502884L;

    struct LineSegment
    {
        sf::Vector2f p1;
        sf::Vector2f p2;
    };

    bool Contains(sf::FloatRect rect, sf::Vector2f point);
    double Distance(sf::Vector2f p1, sf::Vector2f p2);
}
