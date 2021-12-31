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
#include "entity_data.h"
#include "game_math.h"
#include "region.h"
#include "region_definitions.h"

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

    Player();

    void Update(sf::Time elapsed, Region& region);
    void UpdatePlayerState(sf::Vector2i movement_vector);
    void StartAttack(uint16_t attack_angle);
    sf::FloatRect GetBounds();
    util::LineSegment GetSwordLocation();
    definitions::Weapon GetWeapon();
    void SetWeapon(definitions::Weapon new_weapon);
    void Damage(int damage_value);
    bool SpawnProjectile(definitions::Projectile& out_projectile);
    definitions::ItemType UseItem();
    definitions::ItemType ChangeItem(definitions::ItemType item);

    std::shared_ptr<sf::TcpSocket> Socket;
    PlayerStatus Status;
    network::PlayerData Data;

    bool Attacking = false;

private:
    sf::FloatRect getBoundingBox(sf::Vector2f position);
    void handleAttack(sf::Time elapsed, Region& region);

    definitions::PlayerDefinition definition;
    definitions::Weapon weapon;
    definitions::ItemType equipped_item = definitions::ItemType::Medpack;
    sf::Clock projectile_timer;
    int projectiles_fired = 0;
    bool spawn_projectile = false;
    sf::Vector2f velocity;
    double starting_attack_angle;
    double current_attack_angle;

};

} // server
