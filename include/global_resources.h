#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <memory>
#include <functional>

namespace Resources
{
    std::shared_ptr<sf::Texture> AllocTexture(std::string filepath);
    std::shared_ptr<sf::Font> AllocFont(std::string filepath);

    void UpdateEvents(std::map<sf::Event::EventType, std::vector<sf::Event>> events);
    std::vector<sf::Event> GetEvent(sf::Event::EventType type);

    void Log(std::string message);
}

namespace Debug
{
    void DumpBuffer(uint8_t* buffer, int size);
}