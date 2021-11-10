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

    //void Update(sf::Time elapsed);
    void Draw();

    void Unload();

    MenuType CurrentMenu;
    Lobby Lobby;

private:
    void onMouseMove(sf::Event event);
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);
    void onTextEntered(sf::Event event);

    uint64_t mouse_move_id;
    uint64_t mouse_down_id;
    uint64_t mouse_up_id;
    uint64_t text_entered_id;

    std::map<MenuType, Menu> menus;

    void exitGame();
    void setTabOrder();
    void createLobby(bool create);
};

} // client
