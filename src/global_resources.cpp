#include <SFML/Graphics/Texture.hpp>
#include "global_resources.h"
#include <iostream>
#include <map>

using std::cout, std::endl;

namespace {
    std::map<std::string, std::weak_ptr<sf::Texture>> texture_map;
    std::map<std::string, std::weak_ptr<sf::Font>> font_map;
    std::map<sf::Event::EventType, std::vector<sf::Event>> event_map;
}

std::shared_ptr<sf::Texture> Resources::AllocTexture(std::string filepath)
{
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

std::shared_ptr<sf::Font> Resources::AllocFont(std::string filepath)
{
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

void Resources::UpdateEvents(std::map<sf::Event::EventType, std::vector<sf::Event>> events)
{
    event_map = events;
}

std::vector<sf::Event> Resources::GetEvent(sf::Event::EventType type)
{
    if (event_map.find(type) == event_map.end())
    {
        return std::vector<sf::Event>();
    }

    return event_map[type];
}

void Resources::Log(std::string message)
{
    std::cout << message << std::endl;
}
