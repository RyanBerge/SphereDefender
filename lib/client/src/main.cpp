/**************************************************************************************************
 *  File:       main.cpp
 *
 *  Purpose:    The program entry point
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "game_manager.h"
#include "settings.h"
#include <fstream>
#include <filesystem>
#include <iostream>

using std::cout, std::endl;

int main()
{
    std::filesystem::path config_path("config.ini");
    if (std::filesystem::exists(config_path))
    {
        std::ifstream configfile(config_path);
        std::string ip;
        configfile >> ip;
        configfile.close();
        client::Settings::GetInstance().DefaultServerIp = ip;
    }

    client::GameManager::GetInstance().Start();
}
