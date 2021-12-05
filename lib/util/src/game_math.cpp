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

bool Intersects(sf::FloatRect rect, LineSegment line)
{
    // Simple case: out of range
    if ((line.p1.x < rect.left && line.p2.x < rect.left) ||
        (line.p1.x > rect.left + rect.width && line.p2.x > rect.left + rect.width))
    {
        return false;
    }

    if ((line.p1.y < rect.top && line.p2.y < rect.top) ||
        (line.p1.y > rect.top + rect.height && line.p2.y > rect.top + rect.height))
    {
        return false;
    }

    // Simple cases, horizontal and vertical segments
    // vertical
    if (line.p1.x == line.p2.x)
    {
        if (line.p1.x >= rect.left && line.p1.x <= rect.left + rect.width)
        {
            return true;
        }
    }

    // horizontal
    if (line.p1.y == line.p2.y)
    {
        if (line.p1.y >= rect.top && line.p1.y <= rect.top + rect.height)
        {
            return true;
        }
    }


    float slope = (line.p2.y - line.p1.y) / (line.p2.x - line.p1.x);

    // top
    if ((rect.top >= line.p1.y && rect.top <= line.p2.y) ||
        (rect.top >= line.p2.y && rect.top <= line.p1.y))
    {
        float x = (rect.top - line.p2.y) / slope + line.p2.x;
        if (x >= rect.left && x <= rect.left + rect.width)
        {
            return true;
        }
    }

    // bottom
    if ((rect.top + rect.height >= line.p1.y && rect.top + rect.height <= line.p2.y) ||
        (rect.top + rect.height >= line.p2.y && rect.top + rect.height <= line.p1.y))
    {
        float x = (rect.top + rect.height - line.p2.y) / slope + line.p2.x;
        if (x >= rect.left && x <= rect.left + rect.width)
        {
            return true;
        }
    }

    // left
    if ((rect.left >= line.p1.x && rect.left <= line.p2.x) ||
        (rect.left >= line.p2.x && rect.left <= line.p1.x))
    {
        float y = slope * (rect.left - line.p2.x) + line.p2.y;
        if (y >= rect.top && y <= rect.top + rect.height)
        {
            return true;
        }
    }

    // right
    if ((rect.left + rect.width >= line.p1.x && rect.left + rect.width <= line.p2.x) ||
        (rect.left + rect.width >= line.p2.x && rect.left + rect.width <= line.p1.x))
    {
        float y = slope * (rect.left + rect.width - line.p2.x) + line.p2.y;
        if (y >= rect.top && y <= rect.top + rect.height)
        {
            return true;
        }
    }

    return false;
}

bool Intersects(sf::FloatRect rect1, sf::FloatRect rect2)
{
    if (rect1.left > rect2.left + rect2.width)
    {
        return false;
    }

    if (rect1.left + rect1.width < rect2.left)
    {
        return false;
    }

    if (rect1.top > rect2.top + rect2.height)
    {
        return false;
    }

    if (rect1.top + rect1.height < rect2.top)
    {
        return false;
    }

    return true;
}

bool IntersectionPoint(sf::FloatRect rect, LineVector line, sf::Vector2f& out_intersection_point)
{
    // Check if line is even facing in a valid direction
    if (rect.left > line.starting_point.x && line.direction.x <= 0)
    {
        return false;
    }

    if (rect.left + rect.width < line.starting_point.x && line.direction.x >= 0)
    {
        return false;
    }

    if (rect.top > line.starting_point.y && line.direction.y <= 0)
    {
        return false;
    }

    if (rect.top + rect.height < line.starting_point.y && line.direction.y >= 0)
    {
        return false;
    }

    // Check vertical
    if (line.direction.x == 0)
    {
        if (line.starting_point.x >= rect.left && line.starting_point.x <= rect.left + rect.width)
        {
            if (line.direction.y > 0 && rect.top >= line.starting_point.y)
            {
                out_intersection_point = sf::Vector2f{line.starting_point.x, rect.top};
                return true;
            }

            if (line.direction.y < 0 && rect.top + rect.height <= line.starting_point.y)
            {
                out_intersection_point = sf::Vector2f{line.starting_point.x, rect.top + rect.height};
                return true;
            }

            return false;
        }
    }

    // Check horizontal
    if (line.direction.y == 0)
    {
        if (line.starting_point.y >= rect.top && line.starting_point.y <= rect.top + rect.height)
        {
            if (line.direction.x > 0 && rect.left >= line.starting_point.x)
            {
                out_intersection_point = sf::Vector2f{rect.left, line.starting_point.y};
                return true;
            }

            if (line.direction.x < 0 && rect.left + rect.width <= line.starting_point.x)
            {
                out_intersection_point = sf::Vector2f{rect.left + rect.width, line.starting_point.y};
                return true;
            }

            return false;
        }
    }

    float slope = line.direction.y / line.direction.x;

    if (line.direction.x > 0)
    {
        float y = slope * (rect.left - line.starting_point.x) + line.starting_point.y;
        if (y >= rect.top && y <= rect.top + rect.height)
        {
            out_intersection_point = sf::Vector2f{rect.left, y};
            return true;
        }
    }
    else if (line.direction.x < 0)
    {
        float y = slope * (rect.left + rect.width - line.starting_point.x) + line.starting_point.y;
        if (y >= rect.top && y <= rect.top + rect.height)
        {
            out_intersection_point = sf::Vector2f{rect.left + rect.width, y};
            return true;
        }
    }

    if (line.direction.y > 0)
    {
        float x = (rect.top - line.starting_point.y) / slope + line.starting_point.x;
        if (x >= rect.left && x <= rect.left + rect.width)
        {
            out_intersection_point = sf::Vector2f{x, rect.top};
            return true;
        }
    }
    else if (line.direction.y < 0)
    {
        float x = (rect.top + rect.height - line.starting_point.y) / slope + line.starting_point.x;
        if (x >= rect.left && x <= rect.left + rect.width)
        {
            out_intersection_point = sf::Vector2f{x, rect.top + rect.height};
            return true;
        }
    }

    return false;
}

double Distance(sf::Vector2f p1, sf::Vector2f p2)
{
    sf::Vector2f delta = p2 - p1;
    return std::hypot(delta.x, delta.y);
}

} // util
