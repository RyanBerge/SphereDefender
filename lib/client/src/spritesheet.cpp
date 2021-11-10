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
#include "util.h"
#include "game_manager.h"

namespace client {

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string path)
{
    LoadTexture(path);
}

void Spritesheet::Draw()
{
    GameManager::GetInstance().Window.draw(sprite);
}

bool Spritesheet::LoadTexture(std::string path)
{
    texture = util::AllocTexture("assets/" + path);
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

} // client
