#include "main_menu.h"

MainMenu::MainMenu()
{
    Menu splash_screen_menu;
    splash_screen_menu.spritesheets.push_back(Spritesheet("SplashScreen.png"));
    CursorButton splash_screen_start("SplashStart.png");
    splash_screen_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    splash_screen_menu.buttons.push_back(splash_screen_start);
    menus[MenuType::SplashScreen] = splash_screen_menu;

    current_menu = MenuType::SplashScreen;
}

void MainMenu::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    Menu menu = menus[current_menu];

    for (CursorButton& button : menu.buttons)
    {
        button.Update(elapsed, window);
    }
}

void MainMenu::Draw(sf::RenderWindow& window)
{
    Menu menu = menus[current_menu];

    for (Spritesheet& spritesheet : menu.spritesheets)
    {
        spritesheet.Draw(window);
    }

    for (CursorButton& button : menu.buttons)
    {
        button.Draw(window);
    }
}
