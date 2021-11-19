/**************************************************************************************************
 *  File:       settings.h
 *  Class:      Settings
 *
 *  Purpose:    The settings menu and stored options
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "settings.h"
#include "game_manager.h"

namespace client {

Settings::Settings()
{
    // Initialize default settings
    // TODO: Load from Settings file
    Bindings.ScrollLeft = sf::Keyboard::Key::A;
    Bindings.ScrollRight = sf::Keyboard::Key::D;
    Bindings.ScrollUp = sf::Keyboard::Key::W;
    Bindings.ScrollDown = sf::Keyboard::Key::S;
    Bindings.Pause = sf::Keyboard::Key::Escape;

    ServerSettings.ServerPort = 49879;
    WindowResolution = sf::Vector2f{1920, 1280};
    ScrollSpeed = 400;

    frame.LoadTexture("Settings.png");
    frame.SetPosition(410, 200);

    save_button = CursorButton("SettingsSaveButton.png");
    save_button.SetPosition(1220, 770);
    save_button.RegisterLeftMouseUp(std::bind(&Settings::ApplySettings, this));

    settings_view = sf::View(sf::FloatRect(0, 0, WindowResolution.x, WindowResolution.y));
}

Settings& Settings::GetInstance()
{
    static Settings settings;
    return settings;
}


void Settings::Draw()
{
    if (open)
    {
        sf::View old_view = GameManager::GetInstance().Window.getView();
        settings_view.setViewport(old_view.getViewport());
        GameManager::GetInstance().Window.setView(settings_view);

        frame.Draw();
        save_button.Draw();

        GameManager::GetInstance().Window.setView(old_view);
    }
}


void Settings::Open()
{
    open = true;
}

void Settings::Close()
{
    // Anything that needs to be unloaded here
    open = false;
}

void Settings::ApplySettings()
{
    // TODO: May need callbacks hooks here from places that need to know if the settings changed
    Close();
}

} // client
