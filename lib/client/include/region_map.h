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
#include "region_definitions.h"

namespace client {

class RegionMap
{
public:
    RegionMap();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(std::string region);
    void Unload();

    void InitializeRegion(shared::RegionDefinition definition);

private:
    Spritesheet background;
    sf::RectangleShape convoy;

    std::vector<sf::RectangleShape> obstacles;
};

} // client
