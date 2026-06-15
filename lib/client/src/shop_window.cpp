/**************************************************************************************************
 *  File:       shop_window.h
 *  Class:      ShopWindow
 *
 *  Purpose:    Represents a shop window
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <iostream>
#include "shop_window.h"
#include "settings.h"
#include "resources.h"
#include "messaging.h"

using std::cout, std::endl;
using network::ClientMessage;

namespace client {

ShopWindow::ShopWindow()
{
    IsActive = false;
}

ShopWindow::ShopWindow(definitions::Shop shop_definition)
{
    shop_id = shop_definition.id;
    IsActive = false;

    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    font = resources::FontManager::GetFont("Vera");

    frame.setSize(sf::Vector2f{window_resolution.x * 0.95f, window_resolution.y * 0.6f});
    frame.setPosition(sf::Vector2f{window_resolution.x * 0.025f, window_resolution.y * 0.35f});
    frame.setFillColor(sf::Color{125, 125, 125, 240});
    frame.setOutlineColor(sf::Color::Black);
    frame.setOutlineThickness(3);

    for (unsigned i = 0; i < item_frames.size(); ++i)
    {
        item_frames[i].setSize(sf::Vector2f{80, 80});
        item_frames[i].setPosition(sf::Vector2f{frame.getPosition().x + 30 + 120 * i, frame.getPosition().y + 30});
        item_frames[i].setFillColor(sf::Color{175, 175, 175, 255});
        item_frames[i].setOutlineColor(sf::Color::Black);
        item_frames[i].setOutlineThickness(1);
    }

    unsigned i = 0;
    for (auto& item : shop_definition.stock)
    {
        if (item.type == definitions::ItemType::Medpack)
        {
            CursorButton item_button;
            item_button.LoadAnimationData("items/medkit.json");
            sf::Vector2f center;
            center.x = item_frames[i].getPosition().x + (item_frames[i].getSize().x / 2);
            center.y = item_frames[i].getPosition().y + (item_frames[i].getSize().y / 2);
            item_button.SetPosition(center.x, center.y);
            item_button.SetAnimation("Default");
            uint16_t capture_id = shop_id;
            item_button.RegisterLeftMouseDown([capture_id, item](void){ 
                //cout << "Clicked on item with id: " << item.id << "\n";
                if (!item.stale)
                {
                    ClientMessage::BuyItem(resources::GetServerSocket(), capture_id, item.id);
                }
            });

            StockBuyButton buy_button;
            buy_button.button = item_button;
            buy_button.item = item;

            items.push_back(buy_button);
        }

        sf::Text price_text;
        price_text.setString(std::to_string(item.cost));
        price_text.setFont(*font);
        price_text.setCharacterSize(25);
        price_text.setOrigin(sf::Vector2f{price_text.getLocalBounds().getSize().x / 2, 0});
        price_text.setPosition(item_frames[i].getPosition().x + item_frames[i].getSize().x / 2, item_frames[i].getPosition().y + 85);
        price_text.setFillColor(sf::Color::Black);

        prices[i] = price_text;

        ++i;
    }
}

void ShopWindow::Draw()
{
    resources::GetWindow().draw(frame);

    for (auto& item_frame : item_frames)
    {
        resources::GetWindow().draw(item_frame);
    }

    for (auto& price : prices)
    {
        resources::GetWindow().draw(price);
    }

    for (auto& stock_button : items)
    {
        stock_button.button.Draw();
    }
}

void ShopWindow::UpdateStock(definitions::Shop shop)
{
    for (auto& stock_button : items)
    {
        bool found = false;
        for (auto& updated_item : shop.stock)
        {
            if (stock_button.item.id == updated_item.id)
            {
                found = true;
            }
        }
        if (!found)
        {
            stock_button.item.stale = true;
            stock_button.button.SetAnimation("Inactive");
        }
    }
}

void ShopWindow::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    for (auto& stock_button : items)
    {
        if (!stock_button.item.stale)
        {
            stock_button.button.UpdateMouseState(event, CursorButton::State::Down);
        }
}   }

void ShopWindow::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    for (auto& stock_button : items)
    {
        if (!stock_button.item.stale)
        {
            stock_button.button.UpdateMouseState(event, CursorButton::State::Up);
        }
}   }

void ShopWindow::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    for (auto& stock_button : items)
    {
        if (!stock_button.item.stale)
        {
            stock_button.button.UpdateMousePosition(event);
        }
    }
}

} // client
