#pragma once

#include "main_menu.h"
#include "game.h"

class GameManager
{
public:
    GameManager(const GameManager&) = delete;
    GameManager(GameManager&&) = delete;

    GameManager& operator=(const GameManager&) = delete;
    GameManager& operator=(GameManager&&) = delete;

    static GameManager& GetInstance();

    bool Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

    void LoadGame();
private:
    GameManager();

    MainMenu main_menu;
    Game game;
};
