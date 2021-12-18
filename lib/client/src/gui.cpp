/**************************************************************************************************
 *  File:       gui.cpp
 *  Class:      Gui
 *
 *  Purpose:    Represents the in-game GUI
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "gui.h"
#include "settings.h"
#include "resources.h"
#include "game_manager.h"
#include <iostream>
#include <string>

using std::cout, std::cerr, std::endl;

namespace client {

Gui::Gui()
{
    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    font = resources::FontManager::GetFont("Vera");
    sf::FloatRect bounds;

    ui_frame.LoadAnimationData("gui/frame.json");
    ui_frame.SetPosition(0, 0);

    menu_button.LoadAnimationData("gui/menu_button.json");
    menu_button.SetPosition(901, 7);
    menu_button.RegisterLeftMouseUp([this](void){ DisplayMenu(); });

    healthbar.setSize(sf::Vector2f{16, 100});
    healthbar.setOrigin(sf::Vector2f{0, healthbar.getSize().y});
    healthbar.setPosition(sf::Vector2f{window_resolution.x * 0.05f, window_resolution.y * 0.95f});
    healthbar.setFillColor(sf::Color::Green);

    healthbar_frame.LoadAnimationData("gui/healthbar.json");
    healthbar_frame.GetSprite().setOrigin(sf::Vector2f{0, healthbar.getSize().y});
    healthbar_frame.SetPosition(sf::Vector2f{window_resolution.x * 0.05f - 2, window_resolution.y * 0.95f});

    battery_bar_frame.setSize(sf::Vector2f{800, 20});
    battery_bar_frame.setOrigin(sf::Vector2f{0, 10});
    battery_bar_frame.setPosition(sf::Vector2f{40, 40});
    battery_bar_frame.setOutlineColor(sf::Color::Black);
    battery_bar_frame.setOutlineThickness(2);
    battery_bar_frame.setFillColor(sf::Color::Transparent);

    battery_bar.setSize(sf::Vector2f{800, 20});
    battery_bar.setOrigin(sf::Vector2f{0, 10});
    battery_bar.setPosition(sf::Vector2f{40, 40});
    battery_bar.setFillColor(sf::Color{220, 255, 128});
    battery_bar.setScale(sf::Vector2f{0, 1});

    dialog_frame.setSize(sf::Vector2f{window_resolution.x * 0.95f, window_resolution.y * 0.3f});
    dialog_frame.setPosition(sf::Vector2f{window_resolution.x * 0.025f, window_resolution.y * 0.65f});
    dialog_frame.setFillColor(sf::Color{125, 125, 125, 200});
    dialog_frame.setOutlineColor(sf::Color::Black);
    dialog_frame.setOutlineThickness(3);

    dialog_source_text.setFont(*font);
    dialog_source_text.setCharacterSize((Settings::GetInstance().WindowResolution.y * 0.3f) / 8);
    dialog_source_text.setPosition(sf::Vector2f{dialog_frame.getPosition().x + dialog_source_text.getCharacterSize() * 0.5f, dialog_frame.getPosition().y - dialog_source_text.getCharacterSize() * 1.5f});
    dialog_source_text.setFillColor(sf::Color::Black);

    bounds = dialog_frame.getGlobalBounds();
    dialog_prompt_text.setString("Click to continue.");
    dialog_prompt_text.setFont(*font);
    dialog_prompt_text.setCharacterSize((Settings::GetInstance().WindowResolution.y * 0.3f) / 10);
    dialog_prompt_text.setPosition(sf::Vector2f{bounds.left + bounds.width - dialog_prompt_text.getGlobalBounds().width * 1.1f, bounds.top + bounds.height - dialog_prompt_text.getGlobalBounds().height * 3.0f});
    dialog_prompt_text.setFillColor(sf::Color::Black);

    death_text.setFont(*font);
    death_text.setString("You are dead.");
    death_text.setCharacterSize(60);
    bounds = death_text.getGlobalBounds();
    death_text.setPosition(sf::Vector2f{window_resolution.x / 2 - bounds.width / 2, window_resolution.y / 2 - bounds.height / 2});
    death_text.setFillColor(sf::Color::Black);
    death_text.setOutlineColor(sf::Color::White);
    death_text.setOutlineThickness(1);

    death_tint.setSize(window_resolution);
    death_tint.setPosition(0, 0);
    death_tint.setFillColor(sf::Color{100, 100, 100, 230});

    menu.LoadAnimationData("gui/menu.json");
    bounds = menu.GetSprite().getGlobalBounds();
    menu.SetPosition(sf::Vector2f{window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.125f});

    sf::FloatRect reference_bounds = menu.GetSprite().getGlobalBounds();

    resume_button.LoadAnimationData("gui/resume.json");
    bounds = resume_button.GetSprite().getGlobalBounds();
    resume_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.2 - bounds.height / 2);
    resume_button.RegisterLeftMouseUp([this](void){ InMenus = false; });

    save_button.LoadAnimationData("gui/save.json");
    bounds = save_button.GetSprite().getGlobalBounds();
    save_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.35 - bounds.height / 2);

    settings_button.LoadAnimationData("gui/settings.json");
    bounds = settings_button.GetSprite().getGlobalBounds();
    settings_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.5 - bounds.height / 2);

    exit_button.LoadAnimationData("gui/exit.json");
    bounds = exit_button.GetSprite().getGlobalBounds();
    exit_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.8 - bounds.height / 2);
    exit_button.RegisterLeftMouseUp(std::bind(&Gui::exitGame, this));

    GuiView = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));
}

void Gui::Draw()
{
    if (enabled)
    {
        sf::View old_view = resources::GetWindow().getView();
        GuiView.setViewport(old_view.getViewport());
        resources::GetWindow().setView(GuiView);

        ui_frame.Draw();
        menu_button.Draw();

        if (!InDialog)
        {
            resources::GetWindow().draw(healthbar);
            healthbar_frame.Draw();
        }

        resources::GetWindow().draw(battery_bar);
        resources::GetWindow().draw(battery_bar_frame);

        if (Health == 0)
        {
            resources::GetWindow().draw(death_tint);
            resources::GetWindow().draw(death_text);
        }

        if (InDialog)
        {
            resources::GetWindow().draw(dialog_frame);
            for (auto& text : dialog_text)
            {
                resources::GetWindow().draw(text);
            }
            resources::GetWindow().draw(dialog_source_text);
            resources::GetWindow().draw(dialog_prompt_text);
        }

        if (overmap.Active)
        {
            overmap.Draw();
        }

        if (InMenus)
        {
            menu.Draw();
            resume_button.Draw();
            save_button.Draw();
            settings_button.Draw();
            exit_button.Draw();
        }

        resources::GetWindow().setView(old_view);
    }
}

void Gui::Load()
{
    InMenus = false;
    InDialog = false;
    UpdateHealth(100);
}

void Gui::Unload()
{

}

void Gui::SetEnabled(bool new_enabled)
{
    enabled = new_enabled;

    InMenus = false;
    InDialog = false;
    overmap.Active = false;
}

void Gui::UpdateHealth(uint8_t value)
{
    Health = value;
    healthbar.setScale(sf::Vector2f{1, static_cast<float>(Health) / 100});
}

void Gui::UpdateBatteryBar(float battery_level)
{
    battery_bar.setScale(sf::Vector2f{battery_level / 1000, 1});
}

bool Gui::Available()
{
    return !(InMenus || InDialog || overmap.Active);
}

bool Gui::DisableActions()
{
    return InDialog || overmap.Active;
}

void Gui::EscapePressed()
{
    if (overmap.Active)
    {
        overmap.Active = false;
    }
    else if (InDialog)
    {
        advanceDialog();
    }
    else if (InMenus)
    {
        InMenus = false;
    }
    else
    {
        DisplayMenu();
    }
}

void Gui::DisplayMenu()
{
    InMenus = true;
    resume_button.SetAnimation("Up");
    save_button.SetAnimation("Up");
    settings_button.SetAnimation("Up");
    exit_button.SetAnimation("Up");
}

void Gui::DisplayDialog(std::string source, std::vector<std::string> dialog_list)
{
    dialog_text.clear();
    this->dialog = dialog_list;
    current_dialog = 0;
    setDialogText(source, dialog[0]);

    InDialog = true;
}

void Gui::DisplayOvermap()
{
    overmap.Active = true;
}

void Gui::exitGame()
{
    GameManager::GetInstance().Reset();
}

void Gui::setDialogText(std::string source, std::string dialog_block)
{
    dialog_source_text.setString(source);

    dialog_text.clear();
    std::string line = dialog_block;
    float text_overflow;
    float offset = 0;

    do
    {
        sf::Text text;
        text.setString(line);
        text.setFont(*font);
        text.setCharacterSize((Settings::GetInstance().WindowResolution.y * 0.3f) / 6);
        text.setPosition(sf::Vector2f{dialog_frame.getPosition().x + text.getCharacterSize() * 1.5f, dialog_frame.getPosition().y + text.getCharacterSize() + offset});
        text.setFillColor(sf::Color::Black);

        float working_area_width = dialog_frame.getGlobalBounds().width - text.getCharacterSize() * 3;
        text_overflow = text.getGlobalBounds().width - working_area_width;

        if (text_overflow > 0)
        {
            int naive_split = std::floor(working_area_width / text.getGlobalBounds().width * line.size());
            int split = line.substr(0, naive_split).find_last_of(' ');
            text.setString(line.substr(0, split));

            std::string remainder = line.substr(split, line.size());
            if (remainder[0] == ' ')
            {
                line = remainder.substr(1, remainder.size());
            }

            offset += text.getCharacterSize() * 1.1f;
        }

        dialog_text.push_back(text);
    }
    while (text_overflow > 0);
}

void Gui::advanceDialog()
{
    ++current_dialog;
    if (current_dialog == dialog.size())
    {
        InDialog = false;
    }
    else
    {
        setDialogText(dialog_source_text.getString(), dialog[current_dialog]);
    }
}

void Gui::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    if (enabled)
    {
        menu_button.UpdateMousePosition(event);
        if (InMenus)
        {
            resume_button.UpdateMousePosition(event);
            save_button.UpdateMousePosition(event);
            settings_button.UpdateMousePosition(event);
            exit_button.UpdateMousePosition(event);
        }
        else if (overmap.Active)
        {
            overmap.OnMouseMove(event);
        }
    }
}

void Gui::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    if (enabled)
    {
        menu_button.UpdateMouseState(event, CursorButton::State::Down);
        if (InMenus)
        {
            resume_button.UpdateMouseState(event, CursorButton::State::Down);
            save_button.UpdateMouseState(event, CursorButton::State::Down);
            settings_button.UpdateMouseState(event, CursorButton::State::Down);
            exit_button.UpdateMouseState(event, CursorButton::State::Down);
        }
        else if (overmap.Active)
        {
            overmap.OnMouseDown(event);
        }

        if (InDialog && !InMenus && event.button == sf::Mouse::Left)
        {
            advanceDialog();
        }
    }
}

void Gui::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    if (enabled)
    {
        menu_button.UpdateMouseState(event, CursorButton::State::Up);
        if (InMenus)
        {
            resume_button.UpdateMouseState(event, CursorButton::State::Up);
            save_button.UpdateMouseState(event, CursorButton::State::Up);
            settings_button.UpdateMouseState(event, CursorButton::State::Up);
            exit_button.UpdateMouseState(event, CursorButton::State::Up);
        }
        else if (overmap.Active)
        {
            overmap.OnMouseUp(event);
        }
    }
}

void Gui::OnTextEntered(sf::Event::TextEvent event)
{
    (void)event;
    if (enabled)
    {
    }
}

} // client
