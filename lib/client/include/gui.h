/**************************************************************************************************
 *  File:       gui.h
 *  Class:      Gui
 *
 *  Purpose:    Represents the in-game GUI
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "cursor_button.h"
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace client {

class Gui
{
public:
    Gui();

    void Draw();

    void Load();
    void Unload();

    void DisplayMenu();
    void UpdateHealth(uint8_t value);

    sf::View GuiView;
    bool InMenus = false;
    uint8_t Health = 100;

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnTextEntered(sf::Event::TextEvent event);

private:
    void exitGame();

    Spritesheet ui_frame;
    CursorButton menu_button;

    sf::RectangleShape healthbar;
    sf::RectangleShape healthbar_frame;
    sf::RectangleShape death_tint;
    std::shared_ptr<sf::Font> font;
    sf::Text death_text;

    Spritesheet menu;
    CursorButton resume_button;
    CursorButton save_button;
    CursorButton settings_button;
    CursorButton exit_button;
};

} // client
