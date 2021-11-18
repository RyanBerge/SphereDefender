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
#include <atomic>
#include <map>
#include "world_map.h"
#include "gui.h"
#include "player.h"

namespace client {

class Game
{
public:
    Game();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(PlayerState local, std::vector<PlayerState> other_players);
    void Unload();

    void Start(sf::Vector2f spawn_position);

    int GetPlayerCount();
    void UpdatePlayerStates(std::vector<network::PlayerData> player_list);

    sf::View WorldView;

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

    void asyncLoad(PlayerState local, std::vector<PlayerState> other_players);

    std::atomic_bool loaded = false;
    bool menu_open = false;

    Gui gui;
    WorldMap world_map;

    Player local_player;
    std::map<uint16_t, PlayerState> player_states;

    ScrollData scroll_data;
};

} // client
