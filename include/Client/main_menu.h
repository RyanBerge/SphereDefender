#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "cursor_button.h"
#include "textbox.h"
#include <map>
#include <vector>

enum class MenuType
{
    None,
    SplashScreen,
    Main,
    CreateGame,
    JoinGame,
    Lobby,
    Start,
    Exit
};

struct Menu
{
    std::vector<CursorButton> buttons;
    std::vector<Spritesheet> spritesheets;
    std::vector<Textbox> textboxes;
};

class MainMenu
{
public:
    MainMenu();

    void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

    void InitTabOrder();

    MenuType GetCurrentMenu();

private:
    std::map<MenuType, Menu> menus;
    MenuType current_menu;
};
