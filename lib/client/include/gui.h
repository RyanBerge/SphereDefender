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
#include "stash.h"
#include "entity_definitions.h"
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
    void SetOvermapDisplay(bool display);
    void DisplayStash();
    void UpdateHealth(uint8_t value);
    void UpdateBatteryBar(float battery_level);
    void UpdateStash(std::array<definitions::ItemType, 24> items);
    void ChangeItem(definitions::ItemType item);
    void ChangeRegion(uint16_t region_id);
    void MarkInteractables(sf::Vector2f player_position, std::vector<sf::FloatRect> bounds_list);
    bool Available();
    bool DisableActions();
    void EscapePressed();
    void DisplayGatherPlayers(uint16_t player_id, bool start);
    void DisplayVote(uint16_t player_id, uint8_t vote, bool confirmed);

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
    Stash stash;

    sf::Font* font;

    sf::RectangleShape healthbar;
    Spritesheet healthbar_frame;
    Spritesheet inventory_item;
    sf::RectangleShape battery_bar;
    sf::RectangleShape battery_bar_frame;
    sf::RectangleShape death_tint;
    sf::Text death_text;
    Spritesheet interaction_marker;
    sf::Text gather_text;
    bool gathering = false;

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
