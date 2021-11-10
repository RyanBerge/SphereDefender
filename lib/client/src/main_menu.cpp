/**************************************************************************************************
 *  File:       main_menu.cpp
 *  Class:      MainMenu
 *
 *  Purpose:    The primary menu shown to player upon game launch
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <iostream>
#include "main_menu.h"
#include "event_handler.h"
#include "game_manager.h"
#include "util.h"

using std::cout, std::endl;

namespace client {

MainMenu::MainMenu()
{
    // TODO: Move callback registrations to an Initialize function
    mouse_move_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&MainMenu::onMouseMove, this, std::placeholders::_1));
    mouse_down_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&MainMenu::onMouseDown, this, std::placeholders::_1));
    mouse_up_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&MainMenu::onMouseUp, this, std::placeholders::_1));
    text_entered_id = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&MainMenu::onTextEntered, this, std::placeholders::_1));

    std::shared_ptr<sf::Font> font = util::AllocFont("assets/Vera.ttf");

    Menu splash_screen_menu;
    splash_screen_menu.spritesheets.push_back(Spritesheet("SplashScreen.png"));
    CursorButton splash_screen_start("SplashStart.png");
    splash_screen_start.GetSprite().setPosition(sf::Vector2f(472, 650));
    splash_screen_start.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::Main; });
    splash_screen_menu.buttons.push_back(splash_screen_start);
    menus[MenuType::SplashScreen] = splash_screen_menu;
    CurrentMenu = MenuType::SplashScreen;

    Menu main_menu;
    CursorButton create_game_button("CreateGameButton.png");
    create_game_button.GetSprite().setPosition(sf::Vector2f(439, 200));
    create_game_button.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::CreateGame; setTabOrder(); });
    main_menu.buttons.push_back(create_game_button);
    CursorButton join_game_button("JoinGameButton.png");
    join_game_button.GetSprite().setPosition(sf::Vector2f(558, 350));
    join_game_button.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::JoinGame; setTabOrder(); });
    main_menu.buttons.push_back(join_game_button);
    CursorButton exit_game("ExitButton.png");
    exit_game.GetSprite().setPosition(sf::Vector2f(845, 500));
    exit_game.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::Exit; exitGame(); });
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
    create_game_start.RegisterLeftMouseUp([this](void) { createLobby(true); });
    create_game.buttons.push_back(create_game_start);
    Textbox name_box_create("Vera.ttf", sf::Vector2u(800, 75), sf::Color::White, sf::Color::Black);
    name_box_create.SetPosition(sf::Vector2f(200, 200));
    create_game.textboxes.push_back(name_box_create);
    create_game.text.push_back(name_label_text);
    menus[MenuType::CreateGame] = create_game;

    Menu join_game;
    CursorButton connect_button("ConnectButton.png");
    connect_button.GetSprite().setPosition(sf::Vector2f(425, 650));
    connect_button.RegisterLeftMouseUp([this](void) { createLobby(false); });
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
/*
    Menu loading;
    sf::Text loading_text;
    loading_text.setFont(*font);
    loading_text.setString("Loading...");
    loading_text.setCharacterSize(40);
    loading_text.setPosition(sf::Vector2f(500, 300));
    loading_text.setFillColor(sf::Color::White);
    loading.text.push_back(loading_text);
    menus[MenuType::LoadingScreen] = loading;
*/}
/*
void MainMenu::Update(sf::Time elapsed)
{
    (void)elapsed;

    Menu& menu = menus[CurrentMenu];

    if (CurrentMenu == MenuType::Exit)
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

    if (CurrentMenu == MenuType::Lobby)
    {
        Lobby.Update(elapsed, window);
    }
}
*/
void MainMenu::Draw()
{
    Menu& menu = menus[CurrentMenu];

    for (Spritesheet& spritesheet : menu.spritesheets)
    {
        spritesheet.Draw();
    }

    for (CursorButton& button : menu.buttons)
    {
        button.Draw();
    }

    for (Textbox& textbox : menu.textboxes)
    {
        textbox.Draw();
    }

    for (sf::Text& text : menu.text)
    {
        GameManager::GetInstance().Window.draw(text);
    }

    if (CurrentMenu == MenuType::Lobby)
    {
        Lobby.Draw();
    }
}

void MainMenu::Unload()
{
    EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&MainMenu::onMouseMove, this, std::placeholders::_1));
    EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&MainMenu::onMouseDown, this, std::placeholders::_1));
    EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&MainMenu::onMouseUp, this, std::placeholders::_1));
    EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&MainMenu::onTextEntered, this, std::placeholders::_1));
}

void MainMenu::onMouseMove(sf::Event event)
{
    Menu& menu = menus[CurrentMenu];

    for (auto& button : menu.buttons)
    {
        button.UpdateMousePosition(event.mouseMove);
    }
}

void MainMenu::onMouseDown(sf::Event event)
{
    Menu& menu = menus[CurrentMenu];

    for (auto& button : menu.buttons)
    {
        button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
    }

    // Textboxes gain focus only on MouseDown, not MouseUp
    for (auto& textbox : menu.textboxes)
    {
        textbox.UpdateMouseState(event.mouseButton);
    }
}

void MainMenu::onMouseUp(sf::Event event)
{
    Menu& menu = menus[CurrentMenu];

    for (auto& button : menu.buttons)
    {
        button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
    }
}

void MainMenu::onTextEntered(sf::Event event)
{
    Menu& menu = menus[CurrentMenu];

    for (auto& textbox : menu.textboxes)
    {
        if (textbox.UpdateTextInput(event.text))
        {
            // returns true if the event should be consumed by the textbox
            break;
        }
    }
}

void MainMenu::exitGame()
{
    // TODO: Any cleanup code goes here
    GameManager::GetInstance().ExitGame();
}

void MainMenu::setTabOrder()
{
    Menu& menu = menus[CurrentMenu];

    if (menu.textboxes.size() == 0)
    {
        return;
    }

    menu.textboxes[0].HasFocus = true;

    if (menu.textboxes.size() > 1)
    {
        for (unsigned i = 0; i < menu.textboxes.size() - 1; ++i)
        {
            if (i < menu.textboxes.size() - 1)
            {
                menu.textboxes[i].SetTabNext(&menu.textboxes[i + 1]);
            }
        }

        menu.textboxes[menu.textboxes.size() - 1].SetTabNext(&menu.textboxes[0]);
    }
}

void MainMenu::createLobby(bool create)
{
    std::string name = menus[CurrentMenu].textboxes[0].GetText().getString().toAnsiString();
    bool success = false;

    if (create)
    {
        success = Lobby.Create(name);
    }
    else
    {
        std::string ip = menus[CurrentMenu].textboxes[1].GetText().getString().toAnsiString();
        success = Lobby.Join(name, ip);
    }

    if (success)
    {
        CurrentMenu = MenuType::Lobby;
    }
    else
    {
        CurrentMenu = MenuType::Main;
    }
}

} // client
