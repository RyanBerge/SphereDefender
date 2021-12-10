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
#include "game_manager.h"
#include "messaging.h"
#include "settings.h"
#include "resources.h"
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Gui::Gui()
{
    ui_frame.LoadTexture("UiFrame.png");
    ui_frame.SetPosition(0, 0);

    menu_button.LoadAnimationData("gui/menu_button.json");
    menu_button.SetPosition(1803, 14);
    menu_button.RegisterLeftMouseUp([this](void){ DisplayMenu(); });

    healthbar.setSize(sf::Vector2f{32, 200});
    healthbar.setOrigin(sf::Vector2f{0, healthbar.getSize().y});
    healthbar.setPosition(sf::Vector2f{74, 1210});
    healthbar.setFillColor(sf::Color::Green);

    healthbar_frame.LoadAnimationData("gui/healthbar.json");
    healthbar_frame.GetSprite().setOrigin(sf::Vector2f{0, healthbar.getSize().y});
    healthbar_frame.SetPosition(sf::Vector2f{70, 1210});

    battery_bar_frame.setSize(sf::Vector2f{1600, 40});
    battery_bar_frame.setOrigin(sf::Vector2f{0, 20});
    battery_bar_frame.setPosition(sf::Vector2f{80, 80});
    battery_bar_frame.setOutlineColor(sf::Color::Black);
    battery_bar_frame.setOutlineThickness(3);
    battery_bar_frame.setFillColor(sf::Color::Transparent);

    battery_bar.setSize(sf::Vector2f{1600, 40});
    battery_bar.setOrigin(sf::Vector2f{0, 20});
    battery_bar.setPosition(sf::Vector2f{80, 80});
    battery_bar.setFillColor(sf::Color{220, 255, 128});
    battery_bar.setScale(sf::Vector2f{0, 1});

    font = resources::AllocFont("Vera.ttf");
    death_text.setFont(*font);
    death_text.setString("You are dead.");
    death_text.setCharacterSize(120);
    sf::FloatRect bounds = death_text.getGlobalBounds();
    sf::Vector2f resolution = Settings::GetInstance().WindowResolution;
    death_text.setPosition(sf::Vector2f{resolution.x / 2 - bounds.width / 2, resolution.y / 2 - bounds.height / 2});
    death_text.setFillColor(sf::Color::Black);
    death_text.setOutlineColor(sf::Color::White);
    death_text.setOutlineThickness(2);

    death_tint.setSize(Settings::GetInstance().WindowResolution);
    death_tint.setPosition(0, 0);
    death_tint.setFillColor(sf::Color{100, 100, 100, 150});

    menu.LoadTexture("Menu.png");
    menu.SetPosition(613, 95);

    resume_button.LoadAnimationData("gui/resume.json");
    resume_button.SetPosition(707, 232);
    resume_button.RegisterLeftMouseUp([this](void){ InMenus = false; });

    save_button.LoadAnimationData("gui/save.json");
    save_button.SetPosition(707, 373);

    settings_button.LoadAnimationData("gui/settings.json");
    settings_button.SetPosition(707, 515);

    exit_button.LoadAnimationData("gui/exit.json");
    exit_button.SetPosition(707, 798);
    exit_button.RegisterLeftMouseUp(std::bind(&Gui::exitGame, this));

    GuiView = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));
}

void Gui::Draw()
{
    sf::View old_view = GameManager::GetInstance().Window.getView();
    GuiView.setViewport(old_view.getViewport());
    GameManager::GetInstance().Window.setView(GuiView);

    ui_frame.Draw();
    menu_button.Draw();

    GameManager::GetInstance().Window.draw(healthbar);
    healthbar_frame.Draw();

    GameManager::GetInstance().Window.draw(battery_bar);
    GameManager::GetInstance().Window.draw(battery_bar_frame);

    if (Health == 0)
    {
        GameManager::GetInstance().Window.draw(death_tint);
        GameManager::GetInstance().Window.draw(death_text);
    }

    if (InMenus)
    {
        menu.Draw();
        resume_button.Draw();
        save_button.Draw();
        settings_button.Draw();
        exit_button.Draw();
    }

    GameManager::GetInstance().Window.setView(old_view);
}

void Gui::Load()
{
    InMenus = false;
    UpdateHealth(100);
}

void Gui::Unload()
{

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

void Gui::DisplayMenu()
{
    InMenus = true;
    resume_button.SetAnimation("Up");
    save_button.SetAnimation("Up");
    settings_button.SetAnimation("Up");
    exit_button.SetAnimation("Up");
}

void Gui::exitGame()
{
    GameManager::GetInstance().Reset();
}

void Gui::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    menu_button.UpdateMousePosition(event);
    if (InMenus)
    {
        resume_button.UpdateMousePosition(event);
        save_button.UpdateMousePosition(event);
        settings_button.UpdateMousePosition(event);
        exit_button.UpdateMousePosition(event);
    }
}

void Gui::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    menu_button.UpdateMouseState(event, CursorButton::State::Down);
    if (InMenus)
    {
        resume_button.UpdateMouseState(event, CursorButton::State::Down);
        save_button.UpdateMouseState(event, CursorButton::State::Down);
        settings_button.UpdateMouseState(event, CursorButton::State::Down);
        exit_button.UpdateMouseState(event, CursorButton::State::Down);
    }
}

void Gui::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    menu_button.UpdateMouseState(event, CursorButton::State::Up);
    if (InMenus)
    {
        resume_button.UpdateMouseState(event, CursorButton::State::Up);
        save_button.UpdateMouseState(event, CursorButton::State::Up);
        settings_button.UpdateMouseState(event, CursorButton::State::Up);
        exit_button.UpdateMouseState(event, CursorButton::State::Up);
    }
}

void Gui::OnTextEntered(sf::Event::TextEvent event)
{
    (void)event;
}

}

// client
