#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <string>
#include "cursor_button.h"
#include "global_resources.h"
#include "player_states.h"

class Lobby
{
public:
    Lobby();

    bool InitNew(std::string player_name);
    bool InitJoin(std::string player_name, std::string ip);

    void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

    void AddPlayer(PlayerState player);
    void RemovePlayer(uint16_t player_id);
    void ClearPlayers();

    static Lobby* lobby_instance;

private:
    bool owner = false;
    std::shared_ptr<sf::Font> font;
    std::map<uint16_t, sf::Text> player_display_list;
    CursorButton leave_button;
    CursorButton start_button;

    void updatePlayerPositions();
    void onStartPressed();
    void onLeavePressed();
};
