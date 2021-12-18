/**************************************************************************************************
 *  File:       overmap.h
 *  Class:      Overmap
 *
 *  Purpose:    The map that allows players to advance the convoy to different regions
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "cursor_button.h"
#include "SFML/Graphics/RectangleShape.hpp"

namespace client
{

class Overmap
{
public:
    Overmap();

    void Update(sf::Time elapsed);
    void Draw();

    bool Active = false;

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);

private:
    sf::RectangleShape frame;
    sf::RectangleShape map_area;

    std::vector<CursorButton> region_nodes;
};

} // namespace client
