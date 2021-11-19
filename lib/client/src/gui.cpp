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
    action_bar.LoadTexture("ActionBar.png");
    action_bar.SetPosition(192, 1080);

    exit_button = CursorButton("ActionExit.png");
    exit_button.SetPosition(1500, 1145);
    exit_button.RegisterLeftMouseDown(std::bind(&Gui::exitGame, this));

    gui_view = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));
}

void Gui::Draw()
{
    sf::View old_view = GameManager::GetInstance().Window.getView();
    gui_view.setViewport(old_view.getViewport());
    GameManager::GetInstance().Window.setView(gui_view);

    action_bar.Draw();
    exit_button.Draw();

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
    exit_button.UpdateMousePosition(event);
}

void Gui::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    exit_button.UpdateMouseState(event, CursorButton::State::Down);
}

void Gui::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    exit_button.UpdateMouseState(event, CursorButton::State::Up);
}

void Gui::OnTextEntered(sf::Event::TextEvent event)
{
    (void)event;
}

}

// client
