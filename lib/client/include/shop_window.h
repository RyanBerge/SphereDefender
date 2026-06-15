/**************************************************************************************************
 *  File:       shop_window.h
 *  Class:      ShopWindow
 *
 *  Purpose:    Represents a shop window
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <array>
#include "definitions.h"
#include "cursor_button.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace client {

class ShopWindow
{
    struct StockBuyButton
    {
        CursorButton button;
        definitions::ShopItem item;
    };

public:
    ShopWindow();
    ShopWindow(definitions::Shop shop_definition);

    void Draw();

    void UpdateStock(definitions::Shop shop);

    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnMouseMove(sf::Event::MouseMoveEvent event);

    bool IsActive = false;

private:   
    uint16_t shop_id;

    sf::RectangleShape frame;
    std::array<sf::RectangleShape, 6> item_frames;
    std::array<sf::Text, 6> prices;
    std::vector<StockBuyButton> items;
    sf::Font* font;
};

} // client
