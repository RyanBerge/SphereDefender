#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "state_manager.h"
#include "cursor_button.h"
#include <string>

class Lobby
{
public:
    Lobby();

    bool InitNew(std::string player_name);
    bool InitJoin(std::string player_name, std::string ip);

    void Update(sf::Time elapsed, sf::RenderWindow& window);
    void Draw(sf::RenderWindow& window);

private:
    bool owner = false;
    CursorButton leave_button;

    void onLeavePressed();
};
