/**************************************************************************************************
 *  File:       world_map.h
 *  Class:      WorldMap
 *
 *  Purpose:    Represents the world map
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include <SFML/System/Time.hpp>
#include "spritesheet.h"

namespace client {

class WorldMap
{
public:
    WorldMap();

    void Update(sf::Time elapsed);
    void Draw();

    void Load();
    void Unload();

private:
    Spritesheet background;
};

} // client
