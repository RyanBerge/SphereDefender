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
#include "SFML/Graphics/RectangleShape.hpp"
#include <cmath>

namespace util
{
    constexpr double pi = 3.141592653589793238462643383279502884L;
    constexpr float sqrt_2 = 1.42;

    struct LineSegment
    {
        sf::Vector2f p1;
        sf::Vector2f p2;
    };

    struct LineVector
    {
        sf::Vector2f starting_point;
        sf::Vector2f direction;
    };

    bool Contains(sf::FloatRect rect, sf::Vector2f point);
    bool Intersects(sf::FloatRect rect, LineSegment line);
    bool Intersects(sf::FloatRect rect1, sf::FloatRect rect2);
    bool IntersectionPoint(sf::FloatRect rect, LineVector line, sf::Vector2f& out_intersection_point);
    double Distance(sf::Vector2f p1, sf::Vector2f p2);
    sf::Vector2f Normalize(sf::Vector2f vector);
    sf::Vector2f AngleToVector(double angle);
    double VectorToAngle(sf::Vector2f vector);

    sf::RectangleShape CreateLine(sf::Vector2f start, sf::Vector2f finish, sf::Color color, int thickness);

    int GetRandomInt(int min, int max);
    float GetRandomFloat(float min, float max);
}
