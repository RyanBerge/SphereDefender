#include "game_manager.h"

GameManager::GameManager()
{

}

GameManager& GameManager::GetInstance()
{
    static GameManager manager;
    return manager;
}

bool GameManager::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    switch (StateManager::global_state)
    {
        case GlobalState::MainMenu:
        {
            return main_menu.Update(elapsed, window);
        }
        break;
        case GlobalState::Game:
        {
            return game.Update(elapsed, window);
        }
        break;
    }

    return true;
}

void GameManager::Draw(sf::RenderWindow& window)
{
    switch (StateManager::global_state)
    {
        case GlobalState::MainMenu:
        {
            main_menu.Draw(window);
        }
        break;
        case GlobalState::Game:
        {
            return game.Draw(window);
        }
    }
}

void GameManager::LoadGame()
{
    game.Load();
}
