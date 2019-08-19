#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <string>

class Lobby
{
public:
    Lobby();

    void InitNew(std::string player_name);
    void InitExisting(std::string player_name, std::string ip);

    void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

private:

};
