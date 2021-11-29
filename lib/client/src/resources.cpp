/**************************************************************************************************
 *  File:       resources.cpp
 *
 *  Purpose:    Utility functions for managing SFML resources
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <SFML/Graphics/Texture.hpp>
#include "resources.h"
#include <iostream>
#include <map>

using std::cout, std::endl;

namespace client::resources {

namespace {
    std::map<std::string, std::weak_ptr<sf::Texture>> texture_map;
    std::map<std::string, std::weak_ptr<sf::Font>> font_map;
}

std::shared_ptr<sf::Texture> AllocTexture(std::string filename)
{
    std::string filepath = "../assets/" + filename;
    if (texture_map.find(filepath) == texture_map.end() ||
        (texture_map.find(filepath) != texture_map.end() && texture_map[filepath].expired()))
    {
        std::shared_ptr<sf::Texture> texture(new sf::Texture);

        if (texture->loadFromFile(filepath))
        {
            // cout << "Loaded texture: " << filepath << endl;
            texture_map[filepath] = texture;
            return texture;
        }
        else
        {
            cout << "Failed to load texture: " << filepath << std::endl;
            return nullptr;
        }
    }
    else
    {
        return texture_map[filepath].lock();
    }
}

std::shared_ptr<sf::Font> AllocFont(std::string filename)
{
    std::string filepath = "../assets/" + filename;
    if (font_map.find(filepath) == font_map.end() ||
        (font_map.find(filepath) != font_map.end() && font_map[filepath].expired()))
    {
        std::shared_ptr<sf::Font> font(new sf::Font);

        if (font->loadFromFile(filepath))
        {
            // cout << "Loaded font: " << filepath << endl;
            font_map[filepath] = font;
            return font;
        }
        else
        {
            cout << "Failed to load font: " << filepath << std::endl;
            return nullptr;
        }
    }
    else
    {
        return font_map[filepath].lock();
    }
}

} // client::util
