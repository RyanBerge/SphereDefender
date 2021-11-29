/**************************************************************************************************
 *  File:       game_math.cpp
 *
 *  Purpose:    Miscellaneous math functions
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/

#include "game_math.h"
#include <iostream>

using std::cout, std::endl;

namespace util {

bool Contains(sf::FloatRect rect, sf::Vector2f point)
{
    return point.x > rect.left && point.x < rect.left + rect.width &&
           point.y > rect.top && point.y < rect.top + rect.height;
}

double Distance(sf::Vector2f p1, sf::Vector2f p2)
{
    sf::Vector2f delta = p2 - p1;
    return std::sqrt(delta.x * delta.x + delta.y * delta.y);
}

} // util
