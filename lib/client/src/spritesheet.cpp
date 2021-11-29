/**************************************************************************************************
 *  File:       spritesheet.cpp
 *  Class:      Spritesheet
 *
 *  Purpose:    Spritesheet is a wrapper for an sf::Sprite that holds config information that
 *              describes the coordinates on the sheet of each frame.
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "spritesheet.h"
#include "resources.h"
#include "game_manager.h"
#include <iostream>

using std::cout, std::cerr, std::endl;

namespace client {

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string filename)
{
    LoadTexture(filename);
}

Spritesheet::Spritesheet(std::string filename, bool tiled)
{
    LoadTexture(filename);
    SetTiling(tiled);
}

void Spritesheet::Draw()
{
    GameManager::GetInstance().Window.draw(sprite);
    if (texture->isRepeated())
    {
        GameManager::GetInstance().Window.draw(tiled_sprite);
    }
}

bool Spritesheet::LoadTexture(std::string filename)
{
    texture = resources::AllocTexture(filename);
    if (texture == nullptr)
    {
        return false;
    }

    sprite.setTexture(*texture);
    return true;
}

sf::Sprite& Spritesheet::GetSprite()
{
    return sprite;
}

void Spritesheet::SetPosition(float x, float y)
{
    sprite.setPosition(x, y);
    if (texture != nullptr)
    {
        tiled_sprite.setPosition(x + texture->getSize().x / 2, y + texture->getSize().y / 2);
    }
}

void Spritesheet::SetTiling(bool tiled)
{
    texture->setRepeated(tiled);
    tiled_sprite.setTexture(*texture);

    sprite.setTextureRect(sf::Rect{0, 0, 6000, 6000 });
    tiled_sprite.setTextureRect(sf::Rect{0, 0, 6000, 6000 });
}

} // client
