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
#include "cursor_button.h"
#include "messaging.h"

namespace client {

class Lobby
{
public:
    struct LobbyPlayer
    {
        uint16_t id;
        std::string name;
        sf::Text display_text;
    };

    Lobby();

    bool Create(std::string player_name);
    bool Join(std::string player_name, std::string ip);

    //void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw();

    void StartGame();
    void Unload();

    void AssignId(uint16_t id);
    void AddPlayer(network::PlayerData player);
    void RemovePlayer(uint16_t player_id);
    void LeaveLobby();

private:
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);
    uint64_t mouse_down_id;
    uint64_t mouse_up_id;

    void initializeMenu();

    bool owner = false;
    std::shared_ptr<sf::Font> font;
    LobbyPlayer local_player;
    std::vector<LobbyPlayer> player_display_list;
    CursorButton leave_button;
    CursorButton start_button;

    void updatePlayerPositions();
};

} // client
