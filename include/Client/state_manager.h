#pragma once

enum class GlobalState
{
    MainMenu,
    Game
};

enum class MenuType
{
    None,
    SplashScreen,
    Main,
    CreateGame,
    JoinGame,
    Lobby,
    LoadingScreen,
    Exit
};

enum class GameState
{
    None,
    Running
};

namespace StateManager
{
    extern GlobalState global_state;

    namespace MainMenu
    {
        extern MenuType current_menu;
    }

    namespace Game
    {
        extern GameState state;
    }
}
