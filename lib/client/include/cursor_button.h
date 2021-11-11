/**************************************************************************************************
 *  File:       cursor_button.h
 *  Class:      CursorButton
 *
 *  Purpose:    CursorButton represents an input button that can be clicked with the mouse
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Window/Event.hpp>
#include <functional>
#include "spritesheet.h"

namespace client {

class CursorButton
{
public:
    enum class State
    {
        Down,
        Up
    };

    CursorButton();
    CursorButton(std::string filepath);

    //void Update(sf::Time elapsed);
    void Draw();

    void UpdateMousePosition(sf::Event::MouseMoveEvent mouse_event);
    void UpdateMouseState(sf::Event::MouseButtonEvent mouse_event, State state);

    sf::Sprite& GetSprite();
    void SetPosition(float x, float y);

    void RegisterLeftMouseDown(std::function<void(void)> f);
    void RegisterLeftMouseUp(std::function<void(void)> f);

    void RegisterCursorEnter(std::function<void(void)> f);
    void RegisterCursorExit(std::function<void(void)> f);

protected:
    Spritesheet spritesheet;

    void onLeftMouseDown(bool in_bounds);
    void onLeftMouseUp(bool in_bounds);
    void onHoverEnter();
    void onHoverExit();

    bool mouse_hover = false;
    State left_mouse_button_state = State::Up;

    std::vector<std::function<void(void)>> leftMouseDownCallbacks;
    std::vector<std::function<void(void)>> leftMouseUpCallbacks;
    std::vector<std::function<void(void)>> cursorEnterCallbacks;
    std::vector<std::function<void(void)>> cursorExitCallbacks;
};

} // client
