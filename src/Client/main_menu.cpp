#include "main_menu.h"
#include <iostream>

using std::cout, std::endl;

MainMenu::MainMenu()
{
    Menu splash_screen_menu;
    splash_screen_menu.spritesheets.push_back(Spritesheet("SplashScreen.png"));
    CursorButton splash_screen_start("SplashStart.png");
    splash_screen_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    splash_screen_start.RegisterOnClickUp([this](void){ current_menu = MenuType::Main; });
    splash_screen_menu.buttons.push_back(splash_screen_start);
    menus[MenuType::SplashScreen] = splash_screen_menu;
    current_menu = MenuType::SplashScreen;

    Menu main_menu;
    CursorButton create_game_button("CreateGameButton.png");
    create_game_button.GetSprite().setPosition(sf::Vector2f(545, 200));
    create_game_button.RegisterOnClickUp([this](void){ current_menu = MenuType::CreateGame; initTabOrder(); });
    main_menu.buttons.push_back(create_game_button);
    CursorButton exit_game("ExitButton.png");
    exit_game.GetSprite().setPosition(sf::Vector2f(845, 500));
    exit_game.RegisterOnClickUp([this](void){ current_menu = MenuType::Exit; });
    main_menu.buttons.push_back(exit_game);
    menus[MenuType::Main] = main_menu;

    Menu create_game;
    CursorButton create_game_start("SplashStart.png");
    create_game_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    create_game_start.RegisterOnClickUp([this](void){ createLobby(); current_menu = MenuType::Lobby; });
    create_game.buttons.push_back(create_game_start);
    Textbox name_box("Vera.ttf", sf::Vector2u(800, 75), sf::Color::White, sf::Color::Black);
    name_box.SetPosition(sf::Vector2f(200, 200));
    create_game.textboxes.push_back(name_box);
    menus[MenuType::CreateGame] = create_game;

}

void MainMenu::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    Menu& menu = menus[current_menu];

    for (CursorButton& button : menu.buttons)
    {
        button.Update(elapsed, window);
    }

    for (Textbox& textbox : menu.textboxes)
    {
        textbox.Update(elapsed, window);
    }

    if (current_menu == MenuType::Lobby)
    {
        lobby.Update(elapsed, window);
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

    for (Textbox& textbox : menu.textboxes)
    {
        textbox.Draw(window);
    }

    if (current_menu == MenuType::Lobby)
    {
        lobby.Draw(window);
    }
}

void MainMenu::initTabOrder()
{
    Menu& menu = menus[current_menu];

    if (menu.textboxes.size() == 0)
    {
        return;
    }

    menu.textboxes[0].SetFocus(true);
    Textbox* old = &menu.textboxes[0];
    menu.textboxes[menu.textboxes.size() - 1].SetTabNext(old);

    for (unsigned i = 1; i < menu.textboxes.size(); ++i)
    {
        old->SetTabNext(&menu.textboxes[i]);
        old = &menu.textboxes[i];
    }
}

void MainMenu::createLobby()
{
    std::string name = menus[current_menu].textboxes[0].GetText().getString().toAnsiString();
    lobby.InitNew(name);
}
