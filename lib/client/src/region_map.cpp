/**************************************************************************************************
 *  File:       region_map.cpp
 *  Class:      RegionMap
 *
 *  Purpose:    Represents the world map
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region_map.h"
#include "game_manager.h"
#include <SFML/System/Time.hpp>

namespace client {

RegionMap::RegionMap()
{

}

void RegionMap::Update(sf::Time elapsed)
{
    (void)elapsed;
}

void RegionMap::Draw()
{
    background.Draw();

    for (auto& building : buildings)
    {
        GameManager::GetInstance().Window.draw(building);
    }
}

void RegionMap::Load()
{
    background.LoadTexture("Background.png");
    background.SetTiling(true);
    background.SetPosition(-6000, -6000);

    sf::RectangleShape building(sf::Vector2f(300, 800));
    building.setFillColor(sf::Color(115, 150, 180));
    building.setOutlineColor(sf::Color::Black);
    building.setOutlineThickness(5);
    building.setPosition(-600, -400);
    buildings.push_back(building);
}

void RegionMap::Unload() { }

} // client
