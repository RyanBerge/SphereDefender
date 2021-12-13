/**************************************************************************************************
 *  File:       resources.h
 *
 *  Purpose:    Utility functions for managing SFML resources
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <cmath>

namespace client::resources
{
    std::shared_ptr<sf::Texture> AllocTexture(std::string filename);

    class FontManager
    {
    public:
        static sf::Font* GetFont(std::string filename);

    private:
        FontManager();
        ~FontManager();

        std::map<std::string, sf::Font*> font_map;
    };
}
