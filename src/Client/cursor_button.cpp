#include "cursor_button.h"
#include <SFML/Window/Mouse.hpp>
#include <iostream>

using std::cout, std::endl;

CursorButton::CursorButton()
{
}

CursorButton::CursorButton(std::string filepath) : spritesheet{filepath}
{
}

void CursorButton::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    if (window.hasFocus())
    {
        sf::FloatRect bounds = spritesheet.GetSprite().getGlobalBounds();
        auto mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));

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

        if (in_bounds && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !left_mouse_pressed)
        {
            onClickDown();
        }
        else if (in_bounds && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && left_mouse_pressed)
        {
            onClickUp();
        }
    }
}

void CursorButton::Draw(sf::RenderWindow& window)
{
    spritesheet.Draw(window);
}

sf::Sprite& CursorButton::GetSprite()
{
    return spritesheet.GetSprite();
}

void CursorButton::RegisterOnClickDown(std::function<void(void)> f)
{
    onClickDownVector.push_back(f);
}

void CursorButton::RegisterOnClickUp(std::function<void(void)> f)
{
    onClickUpVector.push_back(f);
}

void CursorButton::RegisterOnHoverEnter(std::function<void(void)> f)
{
    onHoverEnterVector.push_back(f);
}

void CursorButton::RegisterOnHoverExit(std::function<void(void)> f)
{
    onHoverExitVector.push_back(f);
}

void CursorButton::onClickUp()
{
    left_mouse_pressed = false;

    for (auto callback : onClickUpVector)
    {
        callback();
    }
}

void CursorButton::onClickDown()
{
    left_mouse_pressed = true;

    for (auto callback : onClickDownVector)
    {
        callback();
    }
}

void CursorButton::onHoverEnter()
{
    mouse_hover = true;

    for (auto callback : onHoverEnterVector)
    {
        callback();
    }
}

void CursorButton::onHoverExit()
{
    mouse_hover = false;

    for (auto callback : onHoverExitVector)
    {
        callback();
    }
}
