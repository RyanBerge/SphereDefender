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
    LoadAnimationData(filepath);
}

void ToggleButton::Toggle() {
    toggled = !toggled;

    if (toggled)
    {
        spritesheet.GetSprite().setColor(sf::Color::Yellow);
    }
    else
    {
        spritesheet.GetSprite().setColor(sf::Color::White);
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

void ToggleButton::RegisterOnToggle(std::function<void(bool)> f)
{
    toggleCallbacks.push_back(f);
}

void ToggleButton::onLeftMouseUp(bool in_bounds)
{
    CursorButton::onLeftMouseUp(in_bounds);

    if (enabled && in_bounds)
    {
        Toggle();
    }
}

} // client
