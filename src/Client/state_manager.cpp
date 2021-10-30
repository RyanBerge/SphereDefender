#include "state_manager.h"

namespace StateManager
{
    GlobalState global_state = GlobalState::MainMenu;

    namespace MainMenu
    {
        MenuType current_menu = MenuType::None;
    }

    namespace Game
    {
        GameState state = GameState::None;
    }
}
