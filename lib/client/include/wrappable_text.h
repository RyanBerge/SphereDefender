/**************************************************************************************************
 *  File:       wrappable_text.h
 *  Class:      WrappableText
 *
 *  Purpose:    General-purpose wrapper around sf::Text to support automatic text wrapping
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/Text.hpp>

namespace client
{

class WrappableText
{
public:
    WrappableText();
    WrappableText(std::string text, sf::FloatRect bounds);

    void Initialize(std::string text, sf::FloatRect bounds);
    void SetText(std::string text);
    void SetBounds(sf::FloatRect bounds);
    void Wrap();

    void Draw();

    sf::Text DisplayText;

private:
    std::string string_text;
    sf::Font* font = nullptr;
    sf::FloatRect bounding_area;
};

} // client
