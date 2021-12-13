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
    leyline.Update(elapsed);
}

void RegionMap::Draw()
{
    background.Draw();
    leyline.Draw();
    GameManager::GetInstance().Window.draw(convoy);

    for (auto& obstacle : obstacles)
    {
        GameManager::GetInstance().Window.draw(obstacle);
    }
}

void RegionMap::Load(std::string region)
{
    background.LoadAnimationData("doodads/background.json");
    background.SetTiling(true);
    background.SetPosition(-6000, -6000);

    InitializeRegion(shared::GetRegionDefinition(region));
}

void RegionMap::Unload() { }

void RegionMap::InitializeRegion(shared::RegionDefinition definition)
{
    if (definition.convoy.orientation == shared::Orientation::North || definition.convoy.orientation == shared::Orientation::South)
    {
        convoy.setSize(sf::Vector2f{definition.convoy.WIDTH, definition.convoy.HEIGHT});
    }
    else if (definition.convoy.orientation == shared::Orientation::East || definition.convoy.orientation == shared::Orientation::West)
    {
        convoy.setSize(sf::Vector2f{definition.convoy.HEIGHT, definition.convoy.WIDTH});
    }

    convoy.setOrigin(convoy.getSize().x / 2, convoy.getSize().y / 2);
    convoy.setPosition(definition.convoy.position);
    convoy.setFillColor(sf::Color(115, 150, 180));
    convoy.setOutlineColor(sf::Color::Black);
    convoy.setOutlineThickness(2);

    leyline.LoadAnimationData("doodads/leyline.json");
    leyline.SetAnimation("Glow");
    leyline.SetPosition(convoy.getPosition().x + 120, convoy.getPosition().y);

    for (auto& obstacle : definition.obstacles)
    {
        sf::RectangleShape rect(obstacle.bounds.getSize());
        rect.setPosition(obstacle.bounds.getPosition());
        rect.setFillColor(sf::Color(100, 100, 100));
        rect.setOutlineColor(sf::Color::Black);
        rect.setOutlineThickness(2);

        obstacles.push_back(rect);
    }
}

} // client
