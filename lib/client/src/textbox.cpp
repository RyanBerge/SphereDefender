/**************************************************************************************************
 *  File:       textbox.h
 *  Class:      Textbox
 *
 *  Purpose:    A GUI entity for entering and holding text
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "textbox.h"
#include "resources.h"
#include "event_handler.h"
#include <functional>
#include <iostream>

namespace client {

using std::cout, std::endl;

namespace {
    const int TEXT_OFFSET = -15;
}

Textbox::Textbox()
{

}

Textbox::Textbox(std::string font_name, sf::Vector2u box_size, sf::Color font_color, sf::Color background_color)
{
    box.setOutlineColor(font_color);
    box.setFillColor(background_color);
    box.setOutlineThickness(box_size.y * 0.05);
    box.setSize(sf::Vector2f(box_size.x, box_size.y));

    font = resources::FontManager::GetFont(font_name);

    if (font != nullptr)
    {
        text.setFont(*font);
        cursor.setFont(*font);
    }

    text.setFillColor(font_color);
    text.setCharacterSize(box_size.y * 0.7);
    text.setString("");

    cursor.setFillColor(font_color);
    cursor.setCharacterSize(box_size.y * 0.7);
    cursor.setString("|");

    SetPosition(sf::Vector2f(0, 0));
}

//void Textbox::Update(sf::Time elapsed) { }

void Textbox::Draw()
{
    resources::GetWindow().draw(box);
    resources::GetWindow().draw(text);

    if (HasFocus)
    {
        resources::GetWindow().draw(cursor);
    }
}

void Textbox::UpdateMouseState(sf::Event::MouseButtonEvent mouse_event)
{
    if (mouse_event.button == sf::Mouse::Button::Left)
    {
        // TODO: Support moving cursor to a particular character on click
        sf::FloatRect bounds = box.getGlobalBounds();
        // TODO: Will probably need a GUI view to map to for buttons with static resources::GetWindow() positions
        sf::Vector2f mouse_position = resources::GetWindow().mapPixelToCoords(sf::Vector2i{mouse_event.x, mouse_event.y});

        bool in_bounds = mouse_position.x >= bounds.left &&
                        mouse_position.x <= bounds.left + bounds.width &&
                        mouse_position.y >= bounds.top &&
                        mouse_position.y <= bounds.top + bounds.height;

        if (HasFocus && !in_bounds)
        {
            HasFocus = false;
        }
        else if (!HasFocus && in_bounds)
        {
            HasFocus = true;
        }
    }
}

bool Textbox::UpdateTextInput(sf::Event::TextEvent text_event)
{
    if (HasFocus)
    {
        // TODO: Magic Numbers
        // TODO: Support arrow keys, delete
        auto character = text_event.unicode;
        if (character > 31 && character < 128)
        {
            sf::Text temp;
            temp.setFont(*font);
            temp.setCharacterSize(text.getCharacterSize());
            temp.setString(text.getString() + character);
            if (temp.getGlobalBounds().width <= box.getSize().x - text.getCharacterSize() * 0.3)
            {
                text.setString(temp.getString());
            }
            ++cursor_index;
        }
        else if (character == 9) // tab
        {
            if (tab_next != nullptr)
            {
                tab_next->HasFocus = true;
                HasFocus = false;
            }
        }
        else if (character == 8) // backspace
        {
            auto temp = text.getString();
            if (temp.getSize() != 0)
            {
                temp = temp.substring(0, text.getString().getSize() - 1);
                text.setString(temp);
                --cursor_index;
            }
        }
        cursor.setPosition(text.getPosition().x + text.getGlobalBounds().width, text.getPosition().y);

        return true;
    }

    return false;
}

sf::Text& Textbox::GetText()
{
    return text;
}

sf::FloatRect Textbox::GetBounds()
{
    return box.getGlobalBounds();
}

void Textbox::SetText(std::string new_text)
{
    text.setString(new_text);
}

void Textbox::SetPosition(sf::Vector2f position)
{
    box.setPosition(position);
    text.setPosition(sf::Vector2f(position.x + text.getCharacterSize() * 0.2, position.y));
    cursor.setPosition(text.getPosition().x + text.getGlobalBounds().width, text.getPosition().y);
}

void Textbox::SetTabNext(Textbox* next)
{
    tab_next = next;
}

} // client
