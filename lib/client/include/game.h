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
#include "region_map.h"
#include "gui.h"
#include "player.h"
#include "enemy.h"

namespace client {

class Game
{
public:
    Game();

    void Update(sf::Time elapsed);
    void Draw();

    void Load(network::PlayerData local, std::vector<network::PlayerData> other_players);
    void Unload();

    void Start(sf::Vector2f spawn_position);

    int GetPlayerCount();
    void UpdatePlayerStates(std::vector<network::PlayerData> player_list);
    void UpdateEnemies(std::vector<network::EnemyData> enemy_list);
    void StartAction(uint16_t player_id, network::PlayerAction action);
    void StartEnemyAction(uint16_t enemy_id, network::EnemyAction action);
    void RemovePlayer(uint16_t player_id);

    sf::View WorldView;

private:
    Gui gui;
    RegionMap region_map;
    Player local_player;

    std::map<uint16_t, Avatar> avatars;
    std::map<uint16_t, Enemy> enemies;

    std::atomic_bool loaded = false;
    bool menu_open = false;

    int zoom_factor;
    float current_zoom;
    float target_zoom;
    float zoom_speed;

    void asyncLoad(network::PlayerData local, std::vector<network::PlayerData> other_players);
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
