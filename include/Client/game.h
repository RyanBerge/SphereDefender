#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <atomic>

class Game
{
public:
    Game();

    bool Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

    void Load();

private:
    void asyncLoad();

    std::atomic_bool loaded = false;

    sf::CircleShape sphere;
};
