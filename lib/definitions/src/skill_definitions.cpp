/**************************************************************************************************
 *  File:       skill_definitions.h
 *
 *  Purpose:    Information about skills
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "skill_definitions.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

using std::cout, std::cerr;

namespace definitions::skills
{

namespace {
SkillDefinitions initialize_skill_definitions()
{
    SkillDefinitions definitions;

    std::filesystem::path path("../data/definitions/skill_definitions.json");
    if (!std::filesystem::exists(path))
    {
        cerr << "File not found: " << path << "\n";
        return definitions;
    }

    try
    {
        std::ifstream file(path);
        nlohmann::json json;
        file >> json;

        definitions.roll.distance = json["roll"]["distance"];
        definitions.roll.duration = json["roll"]["duration"];
        definitions.roll.cooldown = json["roll"]["cooldown"];
    }
    catch (std::exception& e)
    {
        cerr << "Failed to parse spawns file: " << path << "\n";
    }

    return definitions;
}
} // anonymous namespace

SkillDefinitions& GetSkillDefinitions()
{
    static SkillDefinitions skill_definitions = initialize_skill_definitions();
    return skill_definitions;
}

} // namespace definitions
