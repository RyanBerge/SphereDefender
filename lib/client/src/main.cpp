/**************************************************************************************************
 *  File:       main.cpp
 *
 *  Purpose:    The program entry point
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "game_manager.h"

int main()
{
    client::GameManager& game_manager = client::GameManager::GetInstance();
    game_manager.Start();
}
