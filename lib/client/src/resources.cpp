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

using std::cerr, std::endl;

namespace client::resources {

namespace {
    std::map<std::string, std::weak_ptr<sf::Texture>> texture_map;
    std::map<std::string, std::weak_ptr<sf::Texture>> shadow_map;
}

sf::RenderWindow& GetWindow()
{
    static sf::RenderWindow window;
    return window;
}

sf::TcpSocket& GetServerSocket()
{
    static sf::TcpSocket socket;
    return socket;
}

sf::View& GetWorldView()
{
    static sf::View world_view;
    return world_view;
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
            cerr << "Failed to load texture: " << filepath << std::endl;
            return nullptr;
        }
    }
    else
    {
        return texture_map[filepath].lock();
    }
}

std::shared_ptr<sf::Texture> CreateShadow(std::string name, sf::Texture& texture)
{
    if (shadow_map.find(name) == shadow_map.end() ||
        (shadow_map.find(name) != shadow_map.end() && shadow_map[name].expired()))
    {
        std::shared_ptr<sf::Texture> shadow(new sf::Texture);

        sf::Image image = texture.copyToImage();

        for (unsigned x = 0; x < image.getSize().x; ++x)
        {
            for (unsigned y = 0; y < image.getSize().y; ++y)
            {
                sf::Color pixel = image.getPixel(x, y);
                image.setPixel(x, y, sf::Color{0, 0, 0, pixel.a});
            }
        }

        shadow->loadFromImage(image);

        shadow_map[name] = shadow;
        return shadow;
    }
    else
    {
        return shadow_map[name].lock();
    }
}

FontManager::FontManager()
{
    sf::Font* vera = new sf::Font();

    if (!vera->loadFromFile("../assets/Vera.ttf"))
    {
        cerr << "Failed to load font: Vera" << endl;
    }

    font_map["Vera"] = vera;
}

FontManager::~FontManager()
{
    for (auto& element : font_map)
    {
        auto& [name, font] = element;
        if (font != nullptr)
        {
            delete font;
            font = nullptr;
        }
    }

    font_map.clear();
}

sf::Font* FontManager::GetFont(std::string name)
{
    static FontManager font_manager;

    return font_manager.font_map[name];
}

} // client::util
