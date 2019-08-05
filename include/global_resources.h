#pragma once
#include <memory>

namespace Resources
{
    std::shared_ptr<sf::Texture> AllocTexture(std::string filepath);
}
