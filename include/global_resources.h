#pragma once

namespace Resources
{
    sf::Texture* AllocTexture(std::string filepath);
    void FreeTexture(std::string filepath);
}
