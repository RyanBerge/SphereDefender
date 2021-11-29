/**************************************************************************************************
 *  File:       region_map.h
 *  Class:      RegionMap
 *
 *  Purpose:    Represents the world map
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "spritesheet.h"

namespace client {

class RegionMap
{
public:
    RegionMap();

    void Update(sf::Time elapsed);
    void Draw();

    void Load();
    void Unload();

private:
    Spritesheet background;
    std::vector<sf::RectangleShape> buildings;
};

} // client
