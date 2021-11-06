#include "game_manager.h"

int main()
{
    client::GameManager& game_manager = client::GameManager::GetInstance();
    game_manager.Start();
}
