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
    Gui gui;
    WorldMap world_map;
    Player local_player;
    std::map<uint16_t, PlayerState> player_states;

    std::atomic_bool loaded = false;
    bool menu_open = false;

    int zoom_factor = 0;
    float current_zoom = 1;
    float target_zoom = current_zoom;
    float zoom_speed = 0;

    void asyncLoad(PlayerState local, std::vector<PlayerState> other_players);
    void updateScroll(sf::Time elapsed);

    void onMouseMove(sf::Event event);
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);
    void onTextEntered(sf::Event event);
    void onKeyPressed(sf::Event event);
    void onKeyReleased(sf::Event event);
    void onMouseWheel(sf::Event event);

    std::map<sf::Event::EventType, uint64_t> event_id_map;
};

} // client
