#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <string>
#include <memory>

class Textbox
{
public:
    Textbox();
    Textbox(std::string fontpath, sf::Vector2u box_size, sf::Color font_color, sf::Color background_color);

    void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

    void OnTextEntered(sf::Event event);
    void OnMouseDown(sf::Event event, sf::RenderWindow& window);

    sf::Text& GetText();
    void SetPosition(sf::Vector2f position);
    bool GetFocus();
    void SetFocus(bool focus);
    void ApplyDelayedFocus();
    void SetTabNext(Textbox* next);

private:
    std::shared_ptr<sf::Font> font;
    sf::Text text;
    sf::RectangleShape box;
    sf::Text cursor;
    Textbox* tab_next = nullptr;
    bool has_focus = false;
    bool delayed_focus = false;
    int cursor_index = 0;
};
