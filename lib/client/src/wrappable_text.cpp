/**************************************************************************************************
 *  File:       wrappable_text.h
 *  Class:      WrappableText
 *
 *  Purpose:    General-purpose wrapper around sf::Text to support automatic text wrapping
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "wrappable_text.h"
#include "resources.h"

namespace client
{

WrappableText::WrappableText() : WrappableText("", sf::FloatRect{0, 0, 1000, 1000}) { }

WrappableText::WrappableText(std::string text, sf::FloatRect bounds) : string_text{text}, bounding_area{bounds}
{
    font = resources::FontManager::GetFont("Vera");
    DisplayText.setFont(*font);
    DisplayText.setCharacterSize(25);
    DisplayText.setFillColor(sf::Color::White);
    DisplayText.setOutlineColor(sf::Color::Black);
    DisplayText.setOutlineThickness(1);

    Wrap();
}

void WrappableText::Initialize(std::string text, sf::FloatRect bounds)
{
    string_text = text;
    bounding_area = bounds;
    DisplayText.setPosition(sf::Vector2f{bounds.left, bounds.top});
    Wrap();
}

void WrappableText::SetText(std::string text)
{
    string_text = text;
    Wrap();
}

void WrappableText::SetBounds(sf::FloatRect bounds)
{
    bounding_area = bounds;
    Wrap();
}

void WrappableText::Wrap()
{
    DisplayText.setString(string_text);
    if (DisplayText.getGlobalBounds().width <= bounding_area.width)
    {
        return;
    }

    std::string front = DisplayText.getString();
    std::string back = "";
    size_t split_position;

    while (DisplayText.getGlobalBounds().width > bounding_area.width)
    {
        while (DisplayText.getGlobalBounds().width > bounding_area.width)
        {
            std::string string = DisplayText.getString();
            split_position = string.find_last_of(" ");
            front = string.substr(0, split_position);
            back = string.substr(split_position, string.size()) + back;
            DisplayText.setString(front);
        }

        split_position = back.find_first_not_of(" ");
        back = back.substr(split_position, back.size() - split_position);
        std::string new_string = front + "\n" + back;
        back = "";
        DisplayText.setString(new_string);
    }
}

void WrappableText::Draw()
{
    resources::GetWindow().draw(DisplayText);
}

} // client
