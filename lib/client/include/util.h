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
#include <cmath>

namespace client::util
{
    constexpr double pi = 3.141592653589793238462643383279502884L;

    std::shared_ptr<sf::Texture> AllocTexture(std::string filename);
    std::shared_ptr<sf::Font> AllocFont(std::string filename);
}
