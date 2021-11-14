/**************************************************************************************************
 *  File:       world_map.cpp
 *  Class:      WorldMap
 *
 *  Purpose:    Represents the world map
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "world_map.h"
#include <SFML/System/Time.hpp>

namespace client {

WorldMap::WorldMap()
{

}

void WorldMap::Update(sf::Time elapsed)
{
    (void)elapsed;
}

void WorldMap::Draw()
{
    background.Draw();
}

void WorldMap::Load()
{
    background.LoadTexture("Background.png");
    background.SetTiling(true);
    background.SetPosition(-2000, -2000);
}

void WorldMap::Unload() { }

} // client
