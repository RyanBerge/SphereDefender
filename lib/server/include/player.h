/**************************************************************************************************
 *  File:       player_info.h
 *  Class:      Player
 *
 *  Purpose:    The representation of players from the server's point-of-view
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <memory>
#include <list>
#include <queue>
#include "entity_data.h"
#include "game_math.h"
#include "region.h"
#include "definitions.h"

namespace server {

class Player
{
public:
    enum class PlayerStatus
    {
        Uninitialized,
        Disconnected,
        Menus,
        Loading,
        Alive,
        Dead
    };

    struct Vote
    {
        bool voted;
        bool confirmed;
        uint8_t vote;
    };

    Player();

    void Update(sf::Time elapsed, Region& region);
    void UpdatePlayerState(sf::Vector2i movement_vector);
    bool StartAttack(uint16_t attack_angle);
    sf::FloatRect GetBounds();
    util::LineSegment GetSwordLocation();
    definitions::Weapon GetWeapon();
    void SetWeapon(definitions::Weapon new_weapon);
    void Damage(int damage_value);
    bool SpawnProjectile(definitions::Projectile& out_projectile);
    definitions::ItemType UseItem();
    definitions::ItemType ChangeItem(definitions::ItemType item);
    void AddIncomingAttack(definitions::AttackEvent attack);

    std::shared_ptr<sf::TcpSocket> Socket;
    PlayerStatus Status;
    network::PlayerData Data;
    Player::Vote Vote;

    bool Attacking = false;

private:
    sf::FloatRect getBoundingBox(sf::Vector2f position);
    void handleMovement(sf::Time elapsed, Region& region);
    void handleAttack(sf::Time elapsed, Region& region);
    void step(sf::Time elapsed, Region& region);
    void takeStep(sf::Vector2f step, Region& region);
    void processIncomingAttacks();
    void startPlayerAction(network::PlayerAction action);

    definitions::PlayerDefinition definition;
    definitions::Weapon weapon;
    definitions::ItemType equipped_item = definitions::ItemType::Medpack;
    util::Seconds projectile_timer = 0;
    int projectiles_fired = 0;
    bool spawn_projectile = false;
    sf::Vector2f velocity{};
    sf::Vector2f movement_override_vector{};
    util::Seconds movement_override_timer = 0;
    util::Seconds movement_override_time = 0;
    bool movement_override = false;
    util::AngleDegrees starting_attack_angle;
    util::AngleDegrees current_attack_angle;
    util::Seconds attack_timer = 0;

    std::map<uint16_t, util::Seconds> invulnerability_timers;
    std::map<uint16_t, float> invulnerability_windows;

    std::queue<definitions::AttackEvent> attack_events;
};

} // server
