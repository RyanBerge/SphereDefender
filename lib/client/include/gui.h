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

namespace client {

class Gui
{
public:
    Gui();

    void Draw();

    void Load();
    void Unload();

    void OnMouseMove(sf::Event event);
    void OnMouseDown(sf::Event event);
    void OnMouseUp(sf::Event event);
    void OnTextEntered(sf::Event event);

private:
    void exitGame();

    sf::View gui_view;
    Spritesheet action_bar;
    CursorButton exit_button;
};

} // client
