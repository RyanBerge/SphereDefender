/**************************************************************************************************
 *  File:       cursor_button.cpp
 *  Class:      CursorButton
 *
 *  Purpose:    CursorButton represents an input button that can be clicked with the mouse
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <iostream>
#include "cursor_button.h"
#include "game_manager.h"

using std::cout, std::endl;

namespace client {

CursorButton::CursorButton() { }

CursorButton::CursorButton(std::string filepath) : spritesheet{filepath} { }

void CursorButton::Update(sf::Time elapsed)
{
    spritesheet.Update(elapsed);
}

void CursorButton::Draw()
{
    spritesheet.Draw();
}

void CursorButton::LoadAnimationData(std::string filepath)
{
    spritesheet.LoadAnimationData(filepath);
    spritesheet.SetAnimation("Up");
}

void CursorButton::SetAnimation(std::string animation_name)
{
    spritesheet.SetAnimation(animation_name);
}

sf::Sprite& CursorButton::GetSprite()
{
    return spritesheet.GetSprite();
}

void CursorButton::SetPosition(float x, float y)
{
    spritesheet.SetPosition(x, y);
}

void CursorButton::UpdateMousePosition(sf::Event::MouseMoveEvent mouse_event)
{
    sf::FloatRect bounds = spritesheet.GetSprite().getGlobalBounds();
    sf::Vector2f mouse_position = GameManager::GetInstance().Window.mapPixelToCoords(sf::Vector2i{mouse_event.x, mouse_event.y});

    bool in_bounds = mouse_position.x >= bounds.left &&
                    mouse_position.x <= bounds.left + bounds.width &&
                    mouse_position.y >= bounds.top &&
                    mouse_position.y <= bounds.top + bounds.height;

    if (in_bounds && !mouse_hover)
    {
        onHoverEnter();
    }
    else if (!in_bounds && mouse_hover)
    {
        onHoverExit();
    }
}

void CursorButton::UpdateMouseState(sf::Event::MouseButtonEvent mouse_event, State state)
{
    // TODO: Support right-clicks
    if (mouse_event.button == sf::Mouse::Button::Left)
    {
        sf::FloatRect bounds = spritesheet.GetSprite().getGlobalBounds();
        sf::Vector2f mouse_position = GameManager::GetInstance().Window.mapPixelToCoords(sf::Vector2i{mouse_event.x, mouse_event.y});

        bool in_bounds = mouse_position.x >= bounds.left &&
                        mouse_position.x <= bounds.left + bounds.width &&
                        mouse_position.y >= bounds.top &&
                        mouse_position.y <= bounds.top + bounds.height;

        if (state == State::Down && left_mouse_button_state == State::Up)
        {
            onLeftMouseDown(in_bounds);
        }
        else if (state == State::Up && left_mouse_button_state == State::Down)
        {
            onLeftMouseUp(in_bounds);
        }

        left_mouse_button_state = state;
    }
}

void CursorButton::RegisterLeftMouseDown(std::function<void(void)> f)
{
    leftMouseDownCallbacks.push_back(f);
}

void CursorButton::RegisterLeftMouseUp(std::function<void(void)> f)
{
    leftMouseUpCallbacks.push_back(f);
}

void CursorButton::RegisterCursorEnter(std::function<void(void)> f)
{
    cursorEnterCallbacks.push_back(f);
}

void CursorButton::RegisterCursorExit(std::function<void(void)> f)
{
    cursorExitCallbacks.push_back(f);
}

void CursorButton::onLeftMouseDown(bool in_bounds)
{
    spritesheet.SetAnimation("Down");
    if (in_bounds)
    {
        for (auto& callback : leftMouseDownCallbacks)
        {
            callback();
        }
    }
}

void CursorButton::onLeftMouseUp(bool in_bounds)
{
    if (in_bounds)
    {
        for (auto& callback : leftMouseUpCallbacks)
        {
            callback();
        }
    }
}

void CursorButton::onHoverEnter()
{
    mouse_hover = true;

    spritesheet.SetAnimation("Hover");
    for (auto& callback : cursorEnterCallbacks)
    {
        callback();
    }
}

void CursorButton::onHoverExit()
{
    mouse_hover = false;

    spritesheet.SetAnimation("Up");
    for (auto& callback : cursorExitCallbacks)
    {
        callback();
    }
}

} // client
