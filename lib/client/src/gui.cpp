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
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;
#define ServerSocket GameManager::GetInstance().ServerSocket

namespace client {

Gui::Gui()
{
    action_bar.LoadTexture("ActionBar.png");
    action_bar.SetPosition(64, 600);

    exit_button = CursorButton("ActionExit.png");
    exit_button.SetPosition(800, 669.5);
    exit_button.RegisterLeftMouseDown(std::bind(&Gui::exitGame, this));

    // TODO: Move this number to Settings
    gui_view = sf::View(sf::FloatRect(0, 0, 1200, 800));
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

void Gui::OnMouseMove(sf::Event event)
{
    exit_button.UpdateMousePosition(event.mouseMove);
}

void Gui::OnMouseDown(sf::Event event)
{
    exit_button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
}

void Gui::OnMouseUp(sf::Event event)
{
    exit_button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
}

void Gui::OnTextEntered(sf::Event event)
{
    (void)event;
}

}

// client
