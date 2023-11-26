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
#include <random>
#include <ctime>

using std::cout, std::endl;

namespace util {

bool Contains(sf::FloatRect rect, sf::Vector2f point)
{
    return point.x > rect.left && point.x < rect.left + rect.width &&
           point.y > rect.top && point.y < rect.top + rect.height;
}

bool Contains(const std::vector<sf::FloatRect>& rects, sf::Vector2f point)
{
    for (auto& rect : rects)
    {
        if (point.x > rect.left && point.x < rect.left + rect.width &&
            point.y > rect.top && point.y < rect.top + rect.height)
        {
            return true;
        }
    }

    return false;
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

sf::Vector2f Normalize(sf::Vector2f vector)
{
    float magnitude = std::hypot(vector.x, vector.y);
    return sf::Vector2f{vector.x / magnitude, vector.y / magnitude};
}

AngleDegrees ToDegrees(AngleRadians angle)
{
    return angle * (180 / pi);
}

AngleRadians ToRadians(AngleDegrees angle)
{
    return angle * (pi / 180);
}

sf::Vector2f AngleToVector(AngleDegrees angle)
{
    return Normalize(sf::Vector2f{static_cast<float>(std::cos(angle * pi / 180)), static_cast<float>(std::sin(angle * pi / 180))});
}

AngleDegrees VectorToAngle(sf::Vector2f vector)
{
    return std::atan2(vector.y, vector.x) * 180 / pi;
}

sf::Vector2f RotateVector(sf::Vector2f input, AngleDegrees angle)
{
    float sin = std::sin(ToRadians(angle));
    float cos = std::cos(ToRadians(angle));

    sf::Vector2f vector;
    vector.x = (cos * input.x) - (sin * input.y);
    vector.y = (sin * input.x) + (cos * input.y);

    return vector;
}

sf::Vector2f GetPerpendicular(sf::Vector2f input, util::Direction direction)
{
    if (direction == util::Direction::Left)
    {
        return sf::Vector2f{-input.y, input.x};
    }
    else if (direction == util::Direction::Right)
    {
        return sf::Vector2f{input.y, -input.x};
    }
    else
    {
        std::cerr << "Invalid direction input.\n";
        return input;
    }
}

sf::Vector2f TruncateVector(sf::Vector2f input, float magnitude)
{
    float mag = std::hypot(input.x, input.y);
    if (mag < magnitude)
    {
        return input;
    }

    return sf::Vector2f{input.x * (magnitude / mag), input.y * (magnitude / mag)};
}

float Magnitude(sf::Vector2f vector)
{
    return std::hypot(vector.x, vector.y);
}

sf::Vector2f InvertVectorMagnitude(sf::Vector2f input, float max_magnitude)
{
    float mag = std::hypot(input.x, input.y);
    float new_mag = max_magnitude - mag;

    return sf::Vector2f{input.x * (new_mag / mag), input.y * (new_mag / mag)};
}

float DotProduct(sf::Vector2f v1, sf::Vector2f v2)
{
    return (v1.x * v2.x + v1.y * v2.y);
}

AngleDegrees AngleBetween(sf::Vector2f v1, sf::Vector2f v2)
{
    float dot = DotProduct(v1, v2);
    return ToDegrees(std::acos(dot / (std::hypot(v1.x, v1.y) * std::hypot(v2.x, v2.y))));
}

VectorCloud CreateVectorCloud(sf::Vector2f primary)
{
    VectorCloud cloud;
    for (unsigned i = 0; i < cloud.size(); ++i)
    {
        sf::Vector2f vector = primary;
        vector = RotateVector(vector, i * 30);
        cloud[i] = WeightedVector{vector, DotProduct(primary, vector)};
    }

    return cloud;
}

sf::Vector2f CollapseVectorCloud(VectorCloud cloud)
{
    unsigned index = 0;
    float largest_weight = cloud[index].weight;
    for (unsigned i = 1; i < cloud.size(); ++i)
    {
        if (cloud[i].weight > largest_weight)
        {
            largest_weight = cloud[i].weight;
            index = i;
        }
    }

    return cloud[index].vector;
}

util::Direction GetOctalDirection(util::AngleDegrees angle)
{
    angle = std::fmod(angle + 360, 360);
    if (angle <= 23 || angle >= 338)
    {
        return util::Direction::East;
    }
    else if (angle <= 68)
    {
        return util::Direction::Southeast;
    }
    else if (angle <= 113)
    {
        return util::Direction::South;
    }
    else if (angle <= 158)
    {
        return util::Direction::Southwest;
    }
    else if (angle <= 203)
    {
        return util::Direction::West;
    }
    else if (angle <= 248)
    {
        return util::Direction::Northwest;
    }
    else if (angle <= 293)
    {
        return util::Direction::North;
    }
    else
    {
        return util::Direction::Northeast;
    }
}

sf::RectangleShape CreateLine(sf::Vector2f start, sf::Vector2f finish, sf::Color color, int thickness)
{
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f{static_cast<float>(Distance(start, finish)), static_cast<float>(thickness)});
    rect.setOrigin(0, rect.getGlobalBounds().height / 2);
    rect.rotate(VectorToAngle(sf::Vector2f{finish.x - start.x, finish.y - start.y}));
    rect.setFillColor(color);
    rect.setPosition(start);

    return rect;
}

namespace {
    std::random_device random_device;
    std::mt19937 random_generator{random_device()};
}

int GetRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(random_generator);
}

float GetRandomFloat(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(random_generator);
}

sf::Vector2f GetRandomPositionInCone(sf::Vector2f point, float min_distance, float max_distance, AngleDegrees angle, AngleDegrees angle_arc)
{
    float direction = GetRandomFloat(ToRadians(angle - (angle_arc / 2)), ToRadians(angle + (angle_arc / 2)));
    float distance = GetRandomFloat(min_distance, max_distance);

    return sf::Vector2f{distance * std::cos(direction) + point.x, distance * std::sin(direction) + point.y};
}

sf::Vector2f GetRandomPositionFromPoint(sf::Vector2f point, float min_distance, float max_distance)
{
    return GetRandomPositionInCone(point, min_distance, max_distance, 0, 360);
}

} // util

std::ostream &operator<<(std::ostream &output, const sf::Vector2f& vector) {
    output << "(" << vector.x << ", " << vector.y << ")";
    return output;
}
