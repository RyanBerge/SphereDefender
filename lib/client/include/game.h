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
#include <map>
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

    std::map<sf::Event::EventType, uint64_t> event_id_map;

    void asyncLoad();

    std::atomic_bool loaded = false;
    bool menu_open = false;

    sf::View world_view;
    Gui gui;
    WorldMap world_map;

    ScrollData scroll_data;
};

} // client
