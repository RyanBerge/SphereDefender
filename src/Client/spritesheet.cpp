#include "spritesheet.h"
#include "global_resources.h"
#include <iostream>

using std::cout, std::endl;

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string path)
{
    LoadTexture(path);
}

void Spritesheet::Draw(sf::RenderWindow& window)
{
    window.draw(sprite);
}

bool Spritesheet::LoadTexture(std::string path)
{
    texture = Resources::AllocTexture("assets/" + path);
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
