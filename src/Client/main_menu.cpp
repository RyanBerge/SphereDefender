#include "main_menu.h"
#include <iostream>
#include "global_resources.h"

using std::cout, std::endl;

MainMenu::MainMenu()
{
    std::shared_ptr<sf::Font> font = Resources::AllocFont("assets/Vera.ttf");

    Menu splash_screen_menu;
    splash_screen_menu.spritesheets.push_back(Spritesheet("SplashScreen.png"));
    CursorButton splash_screen_start("SplashStart.png");
    splash_screen_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    splash_screen_start.RegisterOnClickUp([this](void){ StateManager::MainMenu::current_menu = MenuType::Main; });
    splash_screen_menu.buttons.push_back(splash_screen_start);
    menus[MenuType::SplashScreen] = splash_screen_menu;
    StateManager::MainMenu::current_menu = MenuType::SplashScreen;

    Menu main_menu;
    CursorButton create_game_button("CreateGameButton.png");
    create_game_button.GetSprite().setPosition(sf::Vector2f(439, 200));
    create_game_button.RegisterOnClickUp([this](void){ StateManager::MainMenu::current_menu = MenuType::CreateGame; initTabOrder(); });
    main_menu.buttons.push_back(create_game_button);
    CursorButton join_game_button("JoinGameButton.png");
    join_game_button.GetSprite().setPosition(sf::Vector2f(558, 350));
    join_game_button.RegisterOnClickUp([this](void){ StateManager::MainMenu::current_menu = MenuType::JoinGame; initTabOrder(); });
    main_menu.buttons.push_back(join_game_button);
    CursorButton exit_game("ExitButton.png");
    exit_game.GetSprite().setPosition(sf::Vector2f(845, 500));
    exit_game.RegisterOnClickUp([this](void){ StateManager::MainMenu::current_menu = MenuType::Exit; exitGame(); });
    main_menu.buttons.push_back(exit_game);
    menus[MenuType::Main] = main_menu;

    sf::Text name_label_text;
    name_label_text.setFont(*font);
    name_label_text.setString("Display Name");
    name_label_text.setCharacterSize(30);
    name_label_text.setPosition(sf::Vector2f(200, 150));
    name_label_text.setFillColor(sf::Color::White);

    sf::Text ip_label_text;
    ip_label_text.setFont(*font);
    ip_label_text.setString("Server Ip Address");
    ip_label_text.setCharacterSize(30);
    ip_label_text.setPosition(sf::Vector2f(200, 350));
    ip_label_text.setFillColor(sf::Color::White);

    Menu create_game;
    CursorButton create_game_start("SplashStart.png");
    create_game_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    create_game_start.RegisterOnClickUp([this](void) {
        if (createLobby(true))
        {
            StateManager::MainMenu::current_menu = MenuType::Lobby;
        }
    });
    create_game.buttons.push_back(create_game_start);
    Textbox name_box_create("Vera.ttf", sf::Vector2u(800, 75), sf::Color::White, sf::Color::Black);
    name_box_create.SetPosition(sf::Vector2f(200, 200));
    create_game.textboxes.push_back(name_box_create);
    create_game.text.push_back(name_label_text);
    menus[MenuType::CreateGame] = create_game;

    Menu join_game;
    CursorButton connect_button("ConnectButton.png");
    connect_button.GetSprite().setPosition(sf::Vector2f(425, 650));
    connect_button.RegisterOnClickUp([this](void) {
        if (createLobby(false))
        {
            StateManager::MainMenu::current_menu = MenuType::Lobby;
        }
    });
    join_game.buttons.push_back(connect_button);
    Textbox name_box_join("Vera.ttf", sf::Vector2u(800, 75), sf::Color::White, sf::Color::Black);
    name_box_join.SetPosition(sf::Vector2f(200, 200));
    join_game.textboxes.push_back(name_box_join);
    Textbox ip_box("Vera.ttf", sf::Vector2u(800, 75), sf::Color::White, sf::Color::Black);
    ip_box.SetPosition(sf::Vector2f(200, 400));
    join_game.textboxes.push_back(ip_box);
    join_game.text.push_back(name_label_text);
    join_game.text.push_back(ip_label_text);
    menus[MenuType::JoinGame] = join_game;

    Menu loading;
    sf::Text loading_text;
    loading_text.setFont(*font);
    loading_text.setString("Loading...");
    loading_text.setCharacterSize(40);
    loading_text.setPosition(sf::Vector2f(500, 300));
    loading_text.setFillColor(sf::Color::White);
    loading.text.push_back(loading_text);
    menus[MenuType::LoadingScreen] = loading;
}

bool MainMenu::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    Menu& menu = menus[StateManager::MainMenu::current_menu];

    if (StateManager::MainMenu::current_menu == MenuType::Exit)
    {
        return false;
    }

    for (CursorButton& button : menu.buttons)
    {
        button.Update(elapsed, window);
    }

    for (Textbox& textbox : menu.textboxes)
    {
        textbox.Update(elapsed, window);
    }

    if (StateManager::MainMenu::current_menu == MenuType::Lobby)
    {
        lobby.Update(elapsed, window);
    }

    return true;
}

void MainMenu::Draw(sf::RenderWindow& window)
{
    Menu& menu = menus[StateManager::MainMenu::current_menu];

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

    for (sf::Text& text : menu.text)
    {
        window.draw(text);
    }

    if (StateManager::MainMenu::current_menu == MenuType::Lobby)
    {
        lobby.Draw(window);
    }
}

void MainMenu::exitGame()
{
    // TODO: Any cleanup code can go here
}

void MainMenu::initTabOrder()
{
    Menu& menu = menus[StateManager::MainMenu::current_menu];

    if (menu.textboxes.size() == 0)
    {
        return;
    }

    menu.textboxes[0].SetFocus(true);

    for (unsigned i = 0; i < menu.textboxes.size() - 1; ++i)
    {
        if (i < menu.textboxes.size() - 1)
        {
            menu.textboxes[i].SetTabNext(&menu.textboxes[i + 1]);
        }
    }
    menu.textboxes[menu.textboxes.size() - 1].SetTabNext(&menu.textboxes[0]);
}

bool MainMenu::createLobby(bool create_new)
{
    std::string name = menus[StateManager::MainMenu::current_menu].textboxes[0].GetText().getString().toAnsiString();

    if (create_new)
    {
        return lobby.InitNew(name);
    }
    else
    {
        std::string ip = menus[StateManager::MainMenu::current_menu].textboxes[1].GetText().getString().toAnsiString();
        return lobby.InitJoin(name, ip);
    }
}
