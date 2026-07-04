/**************************************************************************************************
 *  File:       skills_panel.h
 *  Class:      SkillsPanel
 *
 *  Purpose:    Represents the skills panel
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "toggle_button.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace client {

class SkillsPanel
{
public:
    struct SkillButton
    {
        ToggleButton button;
        sf::Text text;
    };

    SkillsPanel();

    void Draw();

    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnMouseMove(sf::Event::MouseMoveEvent event);

    bool IsActive;

private:
    definitions::PlayerClass player_class;
    sf::Font* font;
    Spritesheet panel;
    sf::RectangleShape background;
    CursorButton x_button;
    sf::Text skill_description;
    sf::Text skill_points;
    std::vector<std::vector<SkillButton>> skills;
};

} // namespace client
