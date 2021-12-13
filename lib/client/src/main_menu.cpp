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
#include "resources.h"
#include "settings.h"

using std::cout, std::endl;

namespace client {

MainMenu::MainMenu()
{
    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    sf::Font* font = resources::FontManager::GetFont("Vera");
    sf::FloatRect bounds;
    sf::FloatRect reference_bounds;

    // ========================================= Splash Screen =========================================
    Menu splash_screen_menu;

    sf::Text splash_game_text;
    splash_game_text.setFont(*font);
    splash_game_text.setCharacterSize(80);
    splash_game_text.setString("Sphere Defender");
    bounds = splash_game_text.getGlobalBounds();
    splash_game_text.setPosition(window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.2 - bounds.height / 2);
    splash_game_text.setFillColor(sf::Color::White);
    splash_screen_menu.text.push_back(splash_game_text);

    Spritesheet splash_wizard("main_menu/splash_wizard.json");
    bounds = splash_wizard.GetSprite().getGlobalBounds();
    reference_bounds = splash_game_text.getGlobalBounds();
    splash_wizard.SetPosition(sf::Vector2f{reference_bounds.left + (reference_bounds.width * 0.7f) - bounds.width / 2, reference_bounds.top + reference_bounds.height});
    splash_screen_menu.spritesheets.push_back(splash_wizard);

    Spritesheet frog;
    frog.LoadAnimationData("main_menu/frog.json");
    frog.SetAnimation("Jump");
    bounds = frog.GetSprite().getGlobalBounds();
    reference_bounds = splash_game_text.getGlobalBounds();
    frog.SetPosition(sf::Vector2f{reference_bounds.left + (reference_bounds.width * 0.2f) - bounds.width / 2, reference_bounds.top + reference_bounds.height + bounds.height * 4});
    splash_screen_menu.spritesheets.push_back(frog);

    CursorButton splash_screen_start("main_menu/start.json");
    bounds = splash_screen_start.GetSprite().getGlobalBounds();
    splash_screen_start.SetPosition(window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.8 - bounds.height / 2);
    splash_screen_start.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::Main; });
    splash_screen_menu.buttons.push_back(splash_screen_start);
    menus[MenuType::SplashScreen] = splash_screen_menu;
    CurrentMenu = MenuType::SplashScreen;

    // ========================================= Main Menu =========================================
    Menu main_menu;

    CursorButton create_button("main_menu/create_lobby.json");
    bounds = create_button.GetSprite().getGlobalBounds();
    create_button.SetPosition(window_resolution.x * 0.95f - bounds.width, window_resolution.y * 0.16 - bounds.height / 2);
    create_button.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::CreateGame; setTabOrder(); });
    main_menu.buttons.push_back(create_button);

    CursorButton join_button("main_menu/join_lobby.json");
    bounds = join_button.GetSprite().getGlobalBounds();
    join_button.SetPosition(window_resolution.x * 0.95f - bounds.width, window_resolution.y * 0.32 - bounds.height / 2);
    join_button.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::JoinGame; setTabOrder(); });
    main_menu.buttons.push_back(join_button);

    CursorButton settings_button("main_menu/settings.json");
    bounds = settings_button.GetSprite().getGlobalBounds();
    settings_button.SetPosition(window_resolution.x * 0.95f - bounds.width, window_resolution.y * 0.48 - bounds.height / 2);
    main_menu.buttons.push_back(settings_button);

    CursorButton exit_button("main_menu/exit.json");
    bounds = exit_button.GetSprite().getGlobalBounds();
    exit_button.SetPosition(window_resolution.x * 0.95f - bounds.width, window_resolution.y * 0.64 - bounds.height / 2);
    exit_button.RegisterLeftMouseUp([this](void){ CurrentMenu = MenuType::Exit; exitGame(); });
    main_menu.buttons.push_back(exit_button);

    menus[MenuType::Main] = main_menu;

    // ========================================= Create/Join Lobby =========================================
    Textbox name_box("Vera", sf::Vector2u(window_resolution.x * 0.6f, window_resolution.y * 0.1), sf::Color::White, sf::Color::Black);
    name_box.SetPosition(sf::Vector2f(window_resolution.x * 0.2f, window_resolution.y * 0.2f));

    Textbox ip_box("Vera", sf::Vector2u(window_resolution.x * 0.6f, window_resolution.y * 0.1), sf::Color::White, sf::Color::Black);
    ip_box.SetPosition(sf::Vector2f(window_resolution.x * 0.2f, window_resolution.y * 0.5f));
    ip_box.SetText(Settings::GetInstance().DefaultServerIp);

    sf::Text name_label;
    name_label.setFont(*font);
    name_label.setString("Display Name");
    name_label.setCharacterSize(20);
    name_label.setPosition(sf::Vector2f{name_box.GetBounds().left, name_box.GetBounds().top - name_label.getGlobalBounds().height * 1.5f});
    name_label.setFillColor(sf::Color::White);

    sf::Text ip_label;
    ip_label.setFont(*font);
    ip_label.setString("Server Ip Address");
    ip_label.setCharacterSize(20);
    ip_label.setPosition(sf::Vector2f{ip_box.GetBounds().left, ip_box.GetBounds().top - ip_label.getGlobalBounds().height * 1.5f});
    ip_label.setFillColor(sf::Color::White);

    CursorButton back_button("main_menu/back.json");
    bounds = back_button.GetSprite().getGlobalBounds();
    back_button.SetPosition(window_resolution.x * 0.2f - bounds.width / 2, window_resolution.y * 0.85f - bounds.height / 2);
    back_button.RegisterLeftMouseUp([this](void) { CurrentMenu = MenuType::Main; });

    CursorButton start_server_button("main_menu/start.json");
    bounds = start_server_button.GetSprite().getGlobalBounds();
    start_server_button.SetPosition(window_resolution.x * 0.8f - bounds.width / 2, window_resolution.y * 0.85f - bounds.height / 2);
    start_server_button.RegisterLeftMouseUp([this](void) { createLobby(true); });

    CursorButton connect_button("main_menu/connect.json");
    bounds = connect_button.GetSprite().getGlobalBounds();
    connect_button.SetPosition(window_resolution.x * 0.8f - bounds.width / 2, window_resolution.y * 0.85f - bounds.height / 2);
    connect_button.RegisterLeftMouseUp([this](void) { createLobby(false); });

    Menu create_lobby;
    create_lobby.textboxes.push_back(name_box);
    create_lobby.text.push_back(name_label);
    create_lobby.buttons.push_back(start_server_button);
    create_lobby.buttons.push_back(back_button);
    menus[MenuType::CreateGame] = create_lobby;

    Menu join_lobby;
    join_lobby.textboxes.push_back(name_box);
    join_lobby.text.push_back(name_label);
    join_lobby.textboxes.push_back(ip_box);
    join_lobby.text.push_back(ip_label);
    join_lobby.buttons.push_back(connect_button);
    join_lobby.buttons.push_back(back_button);
    menus[MenuType::JoinGame] = join_lobby;

    // ========================================= Create/Join Lobby =========================================

    Menu loading;
    sf::Text loading_text;
    loading_text.setFont(*font);
    loading_text.setString("Loading...");
    loading_text.setCharacterSize(40);
    loading_text.setPosition(window_resolution.x * 0.5f - bounds.width * 0.5f, window_resolution.y * 0.5f - bounds.height * 0.5f);
    loading_text.setFillColor(sf::Color::White);
    loading.text.push_back(loading_text);
    menus[MenuType::LoadingScreen] = loading;

    Load();
}

void MainMenu::Update(sf::Time elapsed)
{
    Menu& menu = menus[CurrentMenu];
    for (auto& spritesheet : menu.spritesheets)
    {
        spritesheet.Update(elapsed);
    }
}

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

void MainMenu::Load()
{
    event_id_map[sf::Event::EventType::MouseMoved] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&MainMenu::onMouseMove, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&MainMenu::onMouseDown, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&MainMenu::onMouseUp, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::TextEntered] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&MainMenu::onTextEntered, this, std::placeholders::_1));
}

void MainMenu::Unload()
{
    for (auto& event : event_id_map)
    {
        EventHandler::GetInstance().UnregisterCallback(event.first, event.second);
    }
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
