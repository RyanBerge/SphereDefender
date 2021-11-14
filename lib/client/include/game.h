/**************************************************************************************************
 *  File:       game.h
 *  Class:      Game
 *
 *  Purpose:    The top-level controller for everything in-game
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <atomic>
#include "world_map.h"
#include "gui.h"

namespace client {

class Game
{
public:
    Game();

    void Update(sf::Time elapsed);
    void Draw();

    void Load();
    void Unload();

private:
    struct ScrollData
    {
        int horizontal;
        int vertical;
    };

    void onMouseMove(sf::Event event);
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);
    void onTextEntered(sf::Event event);
    void onKeyPressed(sf::Event event);
    void onKeyReleased(sf::Event event);

    // TODO: Make a map for this
    uint64_t mouse_move_id;
    uint64_t mouse_down_id;
    uint64_t mouse_up_id;
    uint64_t text_entered_id;
    uint64_t key_pressed_id;
    uint64_t key_released_id;

    void asyncLoad();

    std::atomic_bool loaded = false;
    bool menu_open = false;

    sf::View world_view;
    Gui gui;
    WorldMap world_map;

    ScrollData scroll_data;
};

} // client
