#include "player_states.h"
#include "lobby.h"
#include <iostream>


std::map<uint16_t, PlayerState> Players::player_states;
uint16_t Players::owner;


void Players::AddPlayer(PlayerState player_state)
{
    if (Players::player_states.find(player_state.id) != Players::player_states.end())
    {
        std::cerr << "Player joined with an already existing id?" << std::endl;
    }

    Players::player_states[player_state.id] = player_state;

    Lobby::lobby_instance->AddPlayer(player_state);
}

void Players::RemovePlayer(uint16_t player_id)
{
    Players::player_states.erase(player_id);
    Lobby::lobby_instance->RemovePlayer(player_id);
}

void Players::Clear()
{
    player_states.clear();
    Lobby::lobby_instance->ClearPlayers();
}
