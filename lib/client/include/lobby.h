/**************************************************************************************************
 *  File:       lobby.h
 *  Class:      Lobby
 *
 *  Purpose:    The menu that handles game lobbies
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <string>
#include <array>
#include "cursor_button.h"
#include "entity_data.h"

namespace client {

class Lobby
{
public:
    Lobby();

    bool Create(std::string player_name);
    bool Join(std::string player_name, std::string ip);

    void Draw();

    void StartGame();
    void Unload();

    void AssignId(uint16_t id);
    void AddPlayer(network::PlayerData player_data);
    void RemovePlayer(uint16_t player_id);
    void SetPlayerProperties(uint16_t player_id, network::PlayerProperties properties);
    void LeaveLobby();

private:
    struct LobbyPlayer
    {
        network::PlayerData data;
        sf::Text display_text;
        std::array<sf::Text, 2> class_options;
    };

    void onMouseMove(sf::Event event);
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);

    std::map<sf::Event::EventType, uint64_t> event_id_map;

    void initializeMenu();

    bool owner = false;
    sf::Font* font = nullptr;
    LobbyPlayer local_player;
    std::vector<LobbyPlayer> player_display_list;
    CursorButton class_select_left;
    CursorButton class_select_right;
    CursorButton leave_button;
    CursorButton start_button;

    void updatePlayerPositions();
    void scrollClassOption(int displacement);
};

} // client
