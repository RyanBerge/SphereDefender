/**************************************************************************************************
 *  File:       spritesheet.h
 *  Class:      Spritesheet
 *
 *  Purpose:    Spritesheet is a wrapper for an sf::Sprite that holds config information that
 *              describes the coordinates on the sheet of each frame.
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

namespace client {

class Spritesheet
{
public:
    Spritesheet();
    Spritesheet(std::string filepath);
    void Draw();

    bool LoadTexture(std::string filepath);
    sf::Sprite& GetSprite();

private:
    // TODO: Save the name of the file used for the texture so that it can be unloaded and reloaded
    sf::Sprite sprite;
    std::shared_ptr<sf::Texture> texture;
};

} // client