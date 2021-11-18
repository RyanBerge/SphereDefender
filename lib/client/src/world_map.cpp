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
#include "game_manager.h"
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

    for (auto& building : buildings)
    {
        GameManager::GetInstance().Window.draw(building);
    }
}

void WorldMap::Load()
{
    background.LoadTexture("Background.png");
    background.SetTiling(true);
    background.SetPosition(-2000, -2000);

    sf::RectangleShape building(sf::Vector2f(300, 150));
    building.setFillColor(sf::Color(115, 150, 180));
    building.setOutlineColor(sf::Color::Black);
    building.setOutlineThickness(5);
    building.setPosition(-400, -400);
    buildings.push_back(building);
}

void WorldMap::Unload() { }

} // client
