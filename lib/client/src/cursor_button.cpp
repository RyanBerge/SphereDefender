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
#include "resources.h"

using std::cout, std::cerr, std::endl;

namespace client {

CursorButton::CursorButton() { }

CursorButton::CursorButton(std::string filepath)
{
    button_type = ButtonType::Sprite;
    LoadAnimationData(filepath);
}

CursorButton::CursorButton(WrappableText text, sf::Color up, sf::Color hover, sf::Color down, sf::Color disabled) :
                           button_text{text}, animation_up{up}, animation_hover{hover}, animation_down{down}, animation_disabled{disabled}
{
    button_type = ButtonType::Text;
}

void CursorButton::Update(sf::Time elapsed)
{
    if (button_type == ButtonType::Sprite)
    {
        spritesheet.Update(elapsed);
    }
}

void CursorButton::Draw()
{
    switch (button_type)
    {
        case ButtonType::Sprite:
        {
            spritesheet.Draw();
        }
        break;
        case ButtonType::Text:
        {
            button_text.Draw();
        }
        break;
    }
}

void CursorButton::LoadAnimationData(std::string filepath)
{
    button_type = ButtonType::Sprite;
    spritesheet.LoadAnimationData(filepath);
    spritesheet.SetAnimation("Up");
}

void CursorButton::SetAnimation(std::string animation_name)
{
    if (button_type == ButtonType::Sprite)
    {
        spritesheet.SetAnimation(animation_name);
    }
    else
    {
        if (animation_name == "Up")
        {
            button_text.DisplayText.setFillColor(animation_up);
        }
        else if (animation_name == "Hover")
        {
            button_text.DisplayText.setFillColor(animation_hover);
        }
        else if (animation_name == "Down")
        {
            button_text.DisplayText.setFillColor(animation_down);
        }
        else if (animation_name == "Disabled")
        {
            button_text.DisplayText.setFillColor(animation_disabled);
        }
    }
}

void CursorButton::UpdateBoundaryConstraints(sf::FloatRect rect)
{
    if (button_type == ButtonType::Text)
    {
        button_text.SetBounds(rect);
    }
}

void CursorButton::SetEnabled(bool enable)
{
    enabled = enable;
    if (enabled)
    {
        SetAnimation("Up");
    }
    else
    {
        SetAnimation("Disabled");
    }
}

sf::Sprite& CursorButton::GetSprite()
{
    return spritesheet.GetSprite();
}

sf::Transformable& CursorButton::GetTransform()
{
    switch (button_type)
    {
        case ButtonType::Sprite:
        {
            return spritesheet.GetSprite();
        }
        break;
        case ButtonType::Text:
        {
            return button_text.DisplayText;
        }
        break;
        default:
        {
            cerr << "Button has no button type." << endl;
            return spritesheet.GetSprite();
        }
        break;
    }
}

sf::FloatRect CursorButton::GetGlobalBounds()
{
    switch (button_type)
    {
        case ButtonType::Sprite:
        {
            return spritesheet.GetSprite().getGlobalBounds();
        }
        break;
        case ButtonType::Text:
        {
            return button_text.DisplayText.getGlobalBounds();
        }
        break;
        default:
        {
            cerr << "Button has no button type." << endl;
            return spritesheet.GetSprite().getGlobalBounds();
        }
        break;
    }
}

void CursorButton::SetPosition(float x, float y)
{
    GetTransform().setPosition(x, y);
}

void CursorButton::UpdateMousePosition(sf::Event::MouseMoveEvent mouse_event)
{
    sf::FloatRect bounds = GetGlobalBounds();
    sf::Vector2f mouse_position = resources::GetWindow().mapPixelToCoords(sf::Vector2i{mouse_event.x, mouse_event.y});

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
        sf::FloatRect bounds = GetGlobalBounds();
        sf::Vector2f mouse_position = resources::GetWindow().mapPixelToCoords(sf::Vector2i{mouse_event.x, mouse_event.y});

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
    if (enabled)
    {
        if (in_bounds)
        {
            SetAnimation("Down");
            for (auto& callback : leftMouseDownCallbacks)
            {
                callback();
            }
        }
    }
}

void CursorButton::onLeftMouseUp(bool in_bounds)
{
    if (enabled)
    {
        if (in_bounds)
        {
            SetAnimation("Hover");
            for (auto& callback : leftMouseUpCallbacks)
            {
                callback();
            }
        }
        else
        {
            SetAnimation("Up");
        }
    }
}

void CursorButton::onHoverEnter()
{
    if (enabled)
    {
        SetAnimation("Hover");
    }

    mouse_hover = true;

    for (auto& callback : cursorEnterCallbacks)
    {
        callback();
    }
}

void CursorButton::onHoverExit()
{
    if (enabled)
    {
        SetAnimation("Up");
    }

    mouse_hover = false;

    for (auto& callback : cursorExitCallbacks)
    {
        callback();
    }
}

} // client
