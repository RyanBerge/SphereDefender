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

    sf::View GuiView;

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnTextEntered(sf::Event::TextEvent event);

private:
    void exitGame();

    Spritesheet action_bar;
    CursorButton exit_button;
};

} // client
