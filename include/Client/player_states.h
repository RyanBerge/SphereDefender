#pragma once
#include <string>
#include <map>

struct PlayerState
{
    uint16_t id;
    std::string name;
};

namespace Players
{
    extern std::map<uint16_t, PlayerState> player_states;
    extern uint16_t owner;

    void AddPlayer(PlayerState player_state);
    void RemovePlayer(uint16_t player_id);
    void Clear();
}
