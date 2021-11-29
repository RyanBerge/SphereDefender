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
#include "entity_data.h"

namespace client {

class RegionMap
{
public:
    RegionMap();

    void Update(sf::Time elapsed);
    void Draw();

    void Load();
    void Unload();

    void InitializeRegion(network::ConvoyData convoy_data);

private:
    Spritesheet background;
    sf::RectangleShape convoy;
};

} // client
