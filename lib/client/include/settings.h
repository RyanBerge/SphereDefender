/**************************************************************************************************
 *  File:       settings.h
 *  Class:      Settings
 *
 *  Purpose:    The settings menu and stored options
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "cursor_button.h"
#include "server_settings.h"
#include "SFML/Graphics/View.hpp"
#include "SFML/Window/Keyboard.hpp"

namespace client {

class Settings
{
public:
    struct KeyBindings
    {
        sf::Keyboard::Key MoveLeft;
        sf::Keyboard::Key MoveRight;
        sf::Keyboard::Key MoveUp;
        sf::Keyboard::Key MoveDown;

        sf::Keyboard::Key Escape;
        sf::Keyboard::Key Interact;
    };

    Settings();
    static Settings& GetInstance();

    void Draw();

    void Open();
    void Close();
    void ApplySettings();

    KeyBindings Bindings;
    network::ServerSettings ServerSettings;
    std::string DefaultServerIp;
    sf::Vector2f WindowResolution;
    int32_t ScrollSpeed;
    int MaxZoomFactor;
    int MinZoomFactor;

private:
    sf::View settings_view;

    Spritesheet frame;
    CursorButton save_button;

    bool open = false;

};

} // client
