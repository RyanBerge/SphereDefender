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
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Gui::Gui()
{
    ui_frame.LoadTexture("UiFrame.png");
    ui_frame.SetPosition(0, 0);

    menu_button = CursorButton("MenuButton.png");
    menu_button.SetPosition(1803, 14);
    menu_button.RegisterLeftMouseDown([this](void){ InMenus = true; });

    menu.LoadTexture("Menu.png");
    menu.SetPosition(613, 95);

    resume_button = CursorButton("ResumeButton.png");
    resume_button.SetPosition(707, 232);
    resume_button.RegisterLeftMouseDown([this](void){ InMenus = false; });

    save_button = CursorButton("PauseSaveButton.png");
    save_button.SetPosition(707, 373);

    settings_button = CursorButton("PauseSettingsButton.png");
    settings_button.SetPosition(707, 515);

    exit_button = CursorButton("PauseExitButton.png");
    exit_button.SetPosition(707, 798);
    exit_button.RegisterLeftMouseDown(std::bind(&Gui::exitGame, this));

    GuiView = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));
}

void Gui::Draw()
{
    sf::View old_view = GameManager::GetInstance().Window.getView();
    GuiView.setViewport(old_view.getViewport());
    GameManager::GetInstance().Window.setView(GuiView);

    ui_frame.Draw();
    menu_button.Draw();

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

}

void Gui::Unload()
{

}


void Gui::exitGame()
{
    GameManager::GetInstance().Reset();
}

void Gui::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    menu_button.UpdateMousePosition(event);
    resume_button.UpdateMousePosition(event);
    save_button.UpdateMousePosition(event);
    settings_button.UpdateMousePosition(event);
    exit_button.UpdateMousePosition(event);
}

void Gui::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    menu_button.UpdateMouseState(event, CursorButton::State::Down);
    resume_button.UpdateMouseState(event, CursorButton::State::Down);
    save_button.UpdateMouseState(event, CursorButton::State::Down);
    settings_button.UpdateMouseState(event, CursorButton::State::Down);
    exit_button.UpdateMouseState(event, CursorButton::State::Down);
}

void Gui::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    menu_button.UpdateMouseState(event, CursorButton::State::Up);
    resume_button.UpdateMouseState(event, CursorButton::State::Up);
    save_button.UpdateMouseState(event, CursorButton::State::Up);
    settings_button.UpdateMouseState(event, CursorButton::State::Up);
    exit_button.UpdateMouseState(event, CursorButton::State::Up);
}

void Gui::OnTextEntered(sf::Event::TextEvent event)
{
    (void)event;
}

}

// client
