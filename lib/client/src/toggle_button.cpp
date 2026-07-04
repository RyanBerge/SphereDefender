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
#include "toggle_button.h"

using std::cout, std::endl;

namespace client {

ToggleButton::ToggleButton() { }

ToggleButton::ToggleButton(std::string filepath)
{
    mode = Mode::Tint;
    LoadAnimationData(filepath);
}

void ToggleButton::Toggle() {
    toggled = !toggled;

    if (toggled && mode == Mode::Tint)
    {
        spritesheet.GetSprite().setColor(sf::Color::Yellow);
    }
    else if (toggled && mode == Mode::Animation)
    {
        spritesheet.SetAnimation(spritesheet.GetAnimation().name, definitions::AnimationVariant::Active);
    }
    else if (!toggled && mode == Mode::Tint)
    {
        spritesheet.GetSprite().setColor(sf::Color::White);
    }
    else
    {
        spritesheet.SetAnimation(spritesheet.GetAnimation().name, definitions::AnimationVariant::Default);
    }

    for (auto& callback : toggleCallbacks)
    {
        callback(toggled);
    }
}

void ToggleButton::SetToggled(bool toggle_value) {
    if (toggle_value != toggled) {
        Toggle();
    }
}

bool ToggleButton::GetToggled()
{
    return toggled;
}

void ToggleButton::SetMode(Mode new_mode)
{
    mode = new_mode;
}

void ToggleButton::RegisterOnToggle(std::function<void(bool)> f)
{
    toggleCallbacks.push_back(f);
}

void ToggleButton::onLeftMouseUp(bool in_bounds)
{
    if (enabled && in_bounds)
    {
        Toggle();
    }

    CursorButton::onLeftMouseUp(in_bounds);
}

} // client
