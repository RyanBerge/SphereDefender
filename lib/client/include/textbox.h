/**************************************************************************************************
 *  File:       textbox.h
 *  Class:      Textbox
 *
 *  Purpose:    A GUI entity for entering and holding text
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <string>
#include <memory>

namespace client {

class Textbox
{
public:
    Textbox();
    Textbox(std::string font_filename, sf::Vector2u box_size, sf::Color font_color, sf::Color background_color);

    //void Update(sf::Time elapsed);
    void Draw();

    bool HasFocus = false;

    // TODO: Add a callback option for text entered to check for hitting Enter to press buttons
    void UpdateMouseState(sf::Event::MouseButtonEvent mouse_event);
    bool UpdateTextInput(sf::Event::TextEvent text_event);

    sf::Text& GetText();
    void SetPosition(sf::Vector2f position);
    void SetTabNext(Textbox* next);

private:
    void onTextEntered(sf::Event event);
    void onMouseDown(sf::Event event);

    std::shared_ptr<sf::Font> font;
    sf::Text text;
    sf::RectangleShape box;
    sf::Text cursor;
    Textbox* tab_next = nullptr;
    bool has_focus = false;
    bool delayed_focus = false;
    int cursor_index = 0;
};

} // client
