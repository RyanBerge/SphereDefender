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
    GameManager::GetInstance().Window.draw(convoy);
}

void RegionMap::Load()
{
    background.LoadTexture("Background.png");
    background.SetTiling(true);
    background.SetPosition(-6000, -6000);
}

void RegionMap::Unload() { }

void RegionMap::InitializeRegion(network::ConvoyData convoy_data)
{
    if (convoy_data.orientation == network::ConvoyData::Orientation::North || convoy_data.orientation == network::ConvoyData::Orientation::South)
    {
        convoy.setSize(sf::Vector2f{300, 800});
    }
    else if (convoy_data.orientation == network::ConvoyData::Orientation::East || convoy_data.orientation == network::ConvoyData::Orientation::West)
    {
        convoy.setSize(sf::Vector2f{800, 300});
    }

    convoy.setOrigin(convoy.getSize().x / 2, convoy.getSize().y / 2);
    convoy.setPosition(convoy_data.position);
    convoy.setFillColor(sf::Color(115, 150, 180));
    convoy.setOutlineColor(sf::Color::Black);
    convoy.setOutlineThickness(5);
}

} // client
