/**************************************************************************************************
 *  File:       main_menu.h
 *  Class:      MainMenu
 *
 *  Purpose:    The primary menu shown to player upon game launch
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <map>
#include <vector>
#include "cursor_button.h"
#include "textbox.h"
#include "lobby.h"

namespace client {

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
    enum class MenuType
    {
        None,
        SplashScreen,
        Main,
        CreateGame,
        JoinGame,
        Lobby,
        LoadingScreen,
        Exit
    };

    MainMenu();

    void Draw();

    void Load();
    void Unload();

    MenuType CurrentMenu;
    client::Lobby Lobby;

private:
    void onMouseMove(sf::Event event);
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);
    void onTextEntered(sf::Event event);

    std::map<sf::Event::EventType, uint64_t> event_id_map;
    std::shared_ptr<sf::Font> font;

    std::map<MenuType, Menu> menus;

    void exitGame();
    void setTabOrder();
    void createLobby(bool create);
};

} // client
