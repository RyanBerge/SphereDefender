#include <SFML/Graphics/Texture.hpp>
#include "global_resources.h"
#include <iostream>
#include <map>

using std::cout, std::endl;

namespace {

    std::map<std::string, std::weak_ptr<sf::Texture>> texture_map;
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
