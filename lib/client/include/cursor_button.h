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
#include "wrappable_text.h"

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
    CursorButton(WrappableText text, sf::Color up, sf::Color hover, sf::Color down, sf::Color disabled);

    virtual void Update(sf::Time elapsed);
    virtual void Draw();

    virtual void LoadAnimationData(std::string filepath);
    virtual void SetAnimation(std::string animation_name);
    virtual void UpdateBoundaryConstraints(sf::FloatRect rect);

    virtual void SetEnabled(bool enable);

    virtual void UpdateMousePosition(sf::Event::MouseMoveEvent mouse_event);
    virtual void UpdateMouseState(sf::Event::MouseButtonEvent mouse_event, State state);

    virtual sf::Sprite& GetSprite();
    virtual sf::Transformable& GetTransform();
    virtual void SetPosition(float x, float y);
    virtual sf::FloatRect GetGlobalBounds();

    virtual void RegisterLeftMouseDown(std::function<void(void)> f);
    virtual void RegisterLeftMouseUp(std::function<void(void)> f);

    virtual void RegisterCursorEnter(std::function<void(void)> f);
    virtual void RegisterCursorExit(std::function<void(void)> f);

protected:
    enum class ButtonType
    {
        Sprite, Text
    };

    ButtonType button_type = ButtonType::Sprite;

    Spritesheet spritesheet;
    WrappableText button_text;
    sf::Color animation_up;
    sf::Color animation_hover;
    sf::Color animation_down;
    sf::Color animation_disabled;

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
