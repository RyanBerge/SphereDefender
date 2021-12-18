/**************************************************************************************************
 *  File:       gui.h
 *  Class:      Gui
 *
 *  Purpose:    Represents the in-game GUI
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "cursor_button.h"
#include "overmap.h"
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace client {

class Gui
{
public:
    Gui();

    void Draw();

    void Load();
    void Unload();

    void SetEnabled(bool new_enabled);
    void DisplayMenu();
    void DisplayDialog(std::string source, std::vector<std::string> dialog_list);
    void DisplayOvermap();
    void UpdateHealth(uint8_t value);
    void UpdateBatteryBar(float battery_level);
    bool Available();
    bool DisableActions();
    void EscapePressed();

    sf::View GuiView;
    bool InMenus = false;
    bool InDialog = false;
    uint8_t Health = 100;

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);
    void OnTextEntered(sf::Event::TextEvent event);

private:
    void exitGame();
    void setDialogText(std::string source, std::string dialog);
    void advanceDialog();

    bool enabled = true;

    Spritesheet ui_frame;
    CursorButton menu_button;

    Overmap overmap;

    sf::Font* font;

    sf::RectangleShape healthbar;
    Spritesheet healthbar_frame;
    sf::RectangleShape battery_bar;
    sf::RectangleShape battery_bar_frame;
    sf::RectangleShape death_tint;
    sf::Text death_text;

    Spritesheet menu;
    CursorButton resume_button;
    CursorButton save_button;
    CursorButton settings_button;
    CursorButton exit_button;

    sf::RectangleShape dialog_frame;
    std::vector<std::string> dialog;
    std::vector<sf::Text> dialog_text;
    sf::Text dialog_source_text;
    sf::Text dialog_prompt_text;
    unsigned current_dialog = 0;
};

} // client
