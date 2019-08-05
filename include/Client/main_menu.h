#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "cursor_button.h"
#include <map>
#include <vector>

enum class MenuType
{
    None,
    SplashScreen,
    Main
};

struct Menu
{
    std::vector<CursorButton> buttons;
    std::vector<Spritesheet> spritesheets;
};

class MainMenu
{
public:
    MainMenu();

    void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

private:
    std::map<MenuType, Menu> menus;
    MenuType current_menu;
};
