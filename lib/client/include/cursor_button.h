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

    virtual void Update(sf::Time elapsed);
    virtual void Draw();

    virtual void LoadAnimationData(std::string filepath);
    virtual void SetAnimation(std::string animation_name);

    virtual void SetEnabled(bool enable);

    virtual void UpdateMousePosition(sf::Event::MouseMoveEvent mouse_event);
    virtual void UpdateMouseState(sf::Event::MouseButtonEvent mouse_event, State state);

    virtual sf::Sprite& GetSprite();
    virtual void SetPosition(float x, float y);

    virtual void RegisterLeftMouseDown(std::function<void(void)> f);
    virtual void RegisterLeftMouseUp(std::function<void(void)> f);

    virtual void RegisterCursorEnter(std::function<void(void)> f);
    virtual void RegisterCursorExit(std::function<void(void)> f);

protected:
    Spritesheet spritesheet;

    virtual void onLeftMouseDown(bool in_bounds);
    virtual void onLeftMouseUp(bool in_bounds);
    virtual void onHoverEnter();
    virtual void onHoverExit();

    bool enabled = true;
    bool mouse_hover = false;
    State left_mouse_button_state = State::Up;

    std::vector<std::function<void(void)>> leftMouseDownCallbacks;
    std::vector<std::function<void(void)>> leftMouseUpCallbacks;
    std::vector<std::function<void(void)>> cursorEnterCallbacks;
    std::vector<std::function<void(void)>> cursorExitCallbacks;
};

} // client
