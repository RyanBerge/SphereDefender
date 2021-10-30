#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "state_manager.h"
#include "cursor_button.h"
#include "textbox.h"
#include "lobby.h"
#include <map>
#include <vector>

struct Menu
{
    std::vector<CursorButton> buttons;
    std::vector<Spritesheet> spritesheets;
    std::vector<Textbox> textboxes;
    std::vector<sf::Text> text;
};

class MainMenu
{
public:
    MainMenu();

    bool Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

    MenuType GetCurrentMenu();

private:
    std::map<MenuType, Menu> menus;
    Lobby lobby;

    void exitGame();
    void initTabOrder();
    bool createLobby(bool create_new);
};
