/**************************************************************************************************
 *  File:       lobby.h
 *  Class:      MainMenu
 *
 *  Purpose:    The menu that handles game lobbies
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once
#include <SFML/Graphics/Font.hpp>
//#include <SFML/Graphics/Fonts.hpp>
#include <string>
#include "cursor_button.h"
//#include "player_states.h"

namespace client {

class Lobby
{
public:
    Lobby();

    bool Create(std::string player_name);
    bool Join(std::string player_name, std::string ip);

    //void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw();

    //void AddPlayer(PlayerState player);
    //void RemovePlayer(uint16_t player_id);
    //void ClearPlayers();

    //static Lobby* lobby_instance;

private:
    void onMouseDown(sf::Event event);
    void onMouseUp(sf::Event event);
    uint64_t mouse_down_id;
    uint64_t mouse_up_id;

    void initializeMenu();
    void startGame();
    void leaveLobby();

    bool owner = false;
    std::shared_ptr<sf::Font> font;
    //std::map<uint16_t, sf::Text> player_display_list;
    CursorButton leave_button;
    CursorButton start_button;

    //void updatePlayerPositions();
};

} // client
