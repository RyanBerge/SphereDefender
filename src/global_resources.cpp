#include <SFML/Graphics/Texture.hpp>
#include "global_resources.h"
#include <iostream>
#include <map>

using std::cout, std::endl;

namespace {

    struct TextureData
    {
        sf::Texture* texture = nullptr;
        int reference_count = 0;
    };

    std::map<std::string, TextureData> texture_map;
}


sf::Texture* Resources::AllocTexture(std::string filepath)
{
    if (texture_map.find(filepath) != texture_map.end())
    {
        texture_map[filepath].reference_count++;
        return texture_map[filepath].texture;
    }
    else
    {
        TextureData new_texture;
        new_texture.texture = new sf::Texture;
        if (new_texture.texture->loadFromFile(filepath))
        {
            new_texture.reference_count++;
            texture_map[filepath] = new_texture;
            return new_texture.texture;
        }
        else
        {
            cout << "Failed to load texture: " << filepath << std::endl;
            delete new_texture.texture;
            return nullptr;
        }
    }
}

void Resources::FreeTexture(std::string filepath)
{
    if (texture_map.find(filepath) != texture_map.end())
    {
        TextureData texture_data = texture_map[filepath];
        texture_data.reference_count--;
        if (texture_data.reference_count <= 0)
        {
            delete texture_data.texture;
            texture_map.erase(filepath);
        }
    }
    else
    {
        cout << "Failed to free texture not found in texture map: " << filepath << endl;
    }
}
