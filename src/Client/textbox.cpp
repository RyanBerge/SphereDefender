#include "textbox.h"
#include "global_resources.h"
#include <functional>
#include <iostream>

using std::cout, std::endl;

namespace {
    const int TEXT_OFFSET = -15;
}

Textbox::Textbox()
{

}

Textbox::Textbox(std::string fontpath, sf::Vector2u box_size, sf::Color font_color, sf::Color background_color)
{
    box.setOutlineColor(font_color);
    box.setFillColor(background_color);
    box.setOutlineThickness(box_size.y * 0.05);
    box.setSize(sf::Vector2f(box_size.x, box_size.y));

    font = Resources::AllocFont("assets/" + fontpath);

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

void Textbox::Update(sf::Time elapsed, sf::RenderWindow& window)
{
    if (has_focus)
    {
        auto text_events = Resources::GetEvent(sf::Event::TextEntered);
        for (auto& event : text_events)
        {
            OnTextEntered(event);
        }
    }
}

void Textbox::Draw(sf::RenderWindow& window)
{
    window.draw(box);
    window.draw(text);

    if (has_focus)
    {
        window.draw(cursor);
    }
}

void Textbox::OnTextEntered(sf::Event event)
{
    auto character = event.text.unicode;
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
}

sf::Text& Textbox::GetText()
{
    return text;
}

void Textbox::SetPosition(sf::Vector2f position)
{
    box.setPosition(position);
    text.setPosition(sf::Vector2f(position.x + text.getCharacterSize() * 0.2, position.y));
    cursor.setPosition(text.getPosition().x + text.getGlobalBounds().width, text.getPosition().y);
}

bool Textbox::GetFocus()
{
    return has_focus;
}

void Textbox::SetFocus(bool focus)
{
    has_focus = focus;
}

void Textbox::SetTabNext(Textbox* next)
{
    tab_next = next;
}
