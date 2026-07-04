/**************************************************************************************************
 *  File:       settings.h
 *  Class:      Settings
 *
 *  Purpose:    The settings menu and stored options
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <iostream>
#include "settings.h"
#include "resources.h"

using std::cout, std::endl;

namespace client {

Settings::Settings()
{
    // Initialize default settings
    // TODO: Load from Settings file
    Bindings.MoveLeft = sf::Keyboard::Key::A;
    Bindings.MoveRight = sf::Keyboard::Key::D;
    Bindings.MoveUp = sf::Keyboard::Key::W;
    Bindings.MoveDown = sf::Keyboard::Key::S;
    Bindings.Escape = sf::Keyboard::Key::Escape;
    Bindings.Interact = sf::Keyboard::Key::E;
    Bindings.Item = sf::Keyboard::Key::LShift;

    Bindings.UseSkill[SkillBinding::Skill1] = sf::Keyboard::Key::Space;
    Bindings.UseSkill[SkillBinding::Skill2] = sf::Keyboard::Key::Q;
    Bindings.UseSkill[SkillBinding::Skill3] = sf::Keyboard::Key::R;
    Bindings.UseSkill[SkillBinding::Skill4] = sf::Keyboard::Key::F;

    ServerSettings.ServerPort = 49494;
    DefaultServerIp = "127.0.0.1";

    //WindowResolution = sf::Vector2f{1600, 900} * 0.5f;
    WindowResolution = sf::Vector2f{1920, 1080} * 0.5f;
    //WindowResolution = sf::Vector2f{2560, 1440} * 0.5f;

    ScrollSpeed = 600;
    MaxZoomFactor = 10; // 10 increments of +0.1%
    MinZoomFactor = -5; // 5 increments of -0.1%

    AntiAliasing = 8.0f;

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
        sf::View old_view = resources::GetWindow().getView();
        settings_view.setViewport(old_view.getViewport());
        resources::GetWindow().setView(settings_view);

        frame.Draw();
        save_button.Draw();

        resources::GetWindow().setView(old_view);
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

void Settings::BindSkill(definitions::Skill skill)
{
    bool used[4] = {false, false, false, false};
    for (auto& [skill_type, binding] : Bindings.BoundSkills)
    {
        used[(int)binding] = true;
    }

    for (unsigned i = 0; i < 4; ++i)
    {
        if (used[i] == false)
        {
            Bindings.BoundSkills[skill.type] = static_cast<SkillBinding>(i);
            break;
        }
    }
}

} // client
