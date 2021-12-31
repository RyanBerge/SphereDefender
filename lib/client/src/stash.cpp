/**************************************************************************************************
 *  File:       stash.cpp
 *  Class:      Stash
 *
 *  Purpose:    The stash GUI
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "stash.h"
#include "settings.h"
#include "resources.h"
#include "messaging.h"
#include <iostream>

using network::ClientMessage;
using std::cout, std::cerr, std::endl;

namespace client
{

Stash::Stash()
{
    for (unsigned row = 0; row < items.size(); ++row)
    {
        for (unsigned column = 0; column < items[row].size(); ++column)
        {
            auto& item = items[row][column];
            item.LoadAnimationData("gui/inventory_item.json");
            item.SetAnimation("None");
        }
    }

    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    sf::FloatRect bounds = items[0][0].GetSprite().getGlobalBounds();
    sf::Vector2f position{window_resolution.x * 0.15f + bounds.width + 5, window_resolution.y * 0.88f - bounds.height * (items.size() - 1)};

    for (unsigned row = 0; row < items.size(); ++row)
    {
        for (unsigned column = 0; column < items[row].size(); ++column)
        {
            auto& item = items[row][column];
            item.SetPosition(position.x + 70 * column, position.y + 70 * row);
        }
    }
}

void Stash::Update(sf::Time elapsed)
{
    for (auto& row : items)
    {
        for (auto& item : row)
        {
            item.Update(elapsed);
        }
    }
}

void Stash::Draw()
{
    if (Active)
    {
        for (auto& row : items)
        {
            for (auto& item : row)
            {
                item.Draw();
            }
        }
    }
}

void Stash::UpdateItems(std::array<definitions::ItemType, 24> item_array)
{
    for (unsigned i = 0; i < item_array.size(); ++i)
    {
        unsigned row = i / 6;
        unsigned column = i % 6;

        switch (item_array[i])
        {
            case definitions::ItemType::None:
            {
                items[row][column].SetAnimation("None");
            }
            break;
            case definitions::ItemType::Medpack:
            {
                items[row][column].SetAnimation("Medpack");
            }
            break;
        }
    }
}

void Stash::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    (void)event;
}

void Stash::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    if (Active)
    {
        sf::FloatRect bounds;
        bounds.left = items[0][0].GetSprite().getGlobalBounds().left;
        bounds.width = items[0][0].GetSprite().getGlobalBounds().width * items[0].size();
        bounds.top = items[0][0].GetSprite().getGlobalBounds().top;
        bounds.height = items[0][0].GetSprite().getGlobalBounds().height * items.size();

        sf::Vector2f mouse_position = resources::GetWindow().mapPixelToCoords(sf::Vector2i{event.x, event.y});

        if (mouse_position.x >= bounds.left && mouse_position.x <= bounds.left + bounds.width && mouse_position.y >= bounds.top && mouse_position.y <= bounds.top + bounds.height)
        {
            int column = std::floor((mouse_position.x - bounds.left) / items[0][0].GetSprite().getGlobalBounds().width);
            int row = std::floor((mouse_position.y - bounds.top) / items[0][0].GetSprite().getGlobalBounds().height);

            uint8_t index = row * items[0].size() + column;
            ClientMessage::SwapItem(resources::GetServerSocket(), index);
        }
    }
}

void Stash::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    (void)event;
}

} // namespace client
