/**************************************************************************************************
 *  File:       stash.h
 *  Class:      Stash
 *
 *  Purpose:    The stash GUI
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "SFML/System/Time.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "spritesheet.h"
#include "entity_definitions.h"
#include <array>

namespace client
{

class Stash
{
public:
    Stash();

    void Update(sf::Time elapsed);
    void Draw();

    void UpdateItems(std::array<definitions::ItemType, 24> item_array);

    bool Active = false;

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);

private:
    std::array<std::array<Spritesheet, 6>, 4> items;
};

} // namespace client