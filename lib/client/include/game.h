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
#include <mutex>
#include <condition_variable>
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
    std::vector<uint16_t> GetPlayerIds();
    std::string GetPlayerName(uint16_t player_id);
    void SetZone(definitions::Zone zone);
    void UpdatePlayerStates(std::vector<network::PlayerData> player_list);
    void UpdateEnemies(std::vector<network::EnemyData> enemy_list);
    void UpdateProjectiles(std::vector<network::ProjectileData> projectile_list);
    void UpdateBattery(float battery_level);
    void SetPaused(bool paused, network::GuiType gui_type);
    void SetPlayerActionsEnabled(bool enable);
    void StartAction(uint16_t player_id, network::PlayerAction action);
    void ChangeEnemyAction(uint16_t enemy_id, network::EnemyAction action);
    void ChangeItem(definitions::ItemType item);
    void RemovePlayer(uint16_t player_id);
    void ChangeRegion(uint16_t region_id);
    void EnterRegion(sf::Vector2f spawn_position);
    void SetMenuEvent(uint16_t event_id);
    void AdvanceMenuEvent(uint16_t advance_value, bool finish);
    void UpdateStash(std::array<definitions::ItemType, 24> items);
    void DisplayGatherPlayers(uint16_t player_id, bool start);
    void DisplayVote(uint16_t player_id, uint8_t vote, bool confirmed);

    RegionMap region_map;
    bool IsPaused = false;

private:
    Gui gui;
    Player local_player;
    std::map<uint16_t, Avatar> avatars;
    std::map<uint16_t, Enemy> enemies;
    std::vector<sf::RectangleShape> projectiles;
    sf::RectangleShape black_overlay;

    std::atomic_bool loaded = false;
    std::mutex loading_mutex;
    std::condition_variable zone_loaded_condition;
    bool menu_open = false;

    definitions::Zone current_zone;
    std::atomic_bool zone_loaded = false;
    bool leaving_region = false;
    bool entering_region = false;
    uint16_t next_region;

    int zoom_factor;
    float current_zoom;
    float target_zoom;
    float zoom_speed;

    void asyncLoad(network::PlayerData local, std::vector<network::PlayerData> other_players);
    bool isZoneLoaded();
    void updateScroll(sf::Time elapsed);

    enum class LeavingRegionState
    {
        Start, Moving, Fading, Loading, Waiting
    };

    LeavingRegionState leaving_region_state = LeavingRegionState::Start;
    void handleLeavingRegion();

    enum class EnteringRegionState
    {
        Start, Fading, Moving
    };

    EnteringRegionState entering_region_state = EnteringRegionState::Start;
    void handleEnteringRegion();

    bool inCutscene();

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
