/**************************************************************************************************
 *  File:       util.h
 *
 *  Purpose:    Miscellaneous utility functions
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>

namespace client::util
{
    std::shared_ptr<sf::Texture> AllocTexture(std::string filepath);
    std::shared_ptr<sf::Font> AllocFont(std::string filepath);
}
