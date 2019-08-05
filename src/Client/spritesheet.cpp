#include "spritesheet.h"
#include "global_resources.h"

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string path)
{
    LoadTexture(path);
}

Spritesheet::~Spritesheet()
{
    Resources::FreeTexture(filepath);
    texture = nullptr;
}

bool Spritesheet::LoadTexture(std::string path)
{
    filepath = path;
    texture = Resources::AllocTexture(filepath);
    if (texture == nullptr)
    {
        return false;
    }

    sprite.setTexture(*texture);
    return true;
}

void Spritesheet::Draw(sf::RenderWindow& window)
{
    window.draw(sprite);
}
