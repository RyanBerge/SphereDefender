#pragma once

enum class MenuType
{
    None,
    SplashScreen,
    Main,
    CreateGame,
    JoinGame,
    Lobby,
    Start,
    Exit
};

namespace StateManager
{
    namespace MainMenu
    {
        extern MenuType current_menu;
    }
}
