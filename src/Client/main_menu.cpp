#include "main_menu.h"
#include <iostream>

using std::cout, std::endl;

MainMenu::MainMenu()
{
    Menu splash_screen_menu;
    splash_screen_menu.spritesheets.push_back(Spritesheet("SplashScreen.png"));
    CursorButton splash_screen_start("SplashStart.png");
    splash_screen_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    splash_screen_start.RegisterOnClickUp(std::bind(&onSplashStartClick, this));
    splash_screen_menu.buttons.push_back(splash_screen_start);
    menus[MenuType::SplashScreen] = splash_screen_menu;
    current_menu = MenuType::SplashScreen;

    Menu main_menu;
    CursorButton new_game("NewGameButton.png");
    new_game.GetSprite().setPosition(sf::Vector2f(545, 200));
    main_menu.buttons.push_back(new_game);
    CursorButton exit_game("ExitButton.png");
    exit_game.GetSprite().setPosition(sf::Vector2f(845, 500));
    main_menu.buttons.push_back(exit_game);
    menus[MenuType::Main] = main_menu;
}

void MainMenu::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    Menu& menu = menus[current_menu];

    for (CursorButton& button : menu.buttons)
    {
        button.Update(elapsed, window);
    }
}

void MainMenu::Draw(sf::RenderWindow& window)
{
    Menu& menu = menus[current_menu];

    for (Spritesheet& spritesheet : menu.spritesheets)
    {
        spritesheet.Draw(window);
    }

    for (CursorButton& button : menu.buttons)
    {
        button.Draw(window);
    }
}

void MainMenu::onSplashStartClick()
{
    current_menu = MenuType::Main;
}
