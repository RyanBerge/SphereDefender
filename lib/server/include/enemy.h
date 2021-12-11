/**************************************************************************************************
 *  File:       enemy.h
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "SFML/System/Vector2.hpp"
#include "SFML/System/Clock.hpp"
#include <cstdint>
#include <list>
#include "entity_data.h"
#include "player_info.h"
#include "region_definitions.h"

namespace server {

class Enemy
{
public:
    enum class Behavior
    {
        Moving,
        Feeding,
        Hunting,
        Dead
    };

    enum class Action
    {
        None,
        Attacking,
        Knockback,
        Stunned
    };

    Enemy();

    void Update(sf::Time elapsed, shared::ConvoyDefinition convoy, std::vector<PlayerInfo>& players, std::vector<sf::FloatRect> obstacles);
    void WeaponHit(uint16_t player_id, uint8_t damage, PlayerInfo::WeaponKnockback knockback, sf::Vector2f hit_vector, std::vector<PlayerInfo>& players);

    Behavior GetBehavior();
    Action GetAction();
    sf::FloatRect GetBounds();
    int GetSiphonRate();

    network::EnemyData Data;
    bool Despawn = false;

private:
    float movement_speed = 210;
    float attack_range = 35 + 35;
    float feeding_range = 35;
    float attack_distance = 60; // pixels
    float attack_duration = 0.25; // seconds
    float attack_cooldown = 0.5; // seconds
    int attack_damage = 30;
    sf::Vector2f base_size{50, 50};

    Behavior current_behavior;
    Action current_action;

    uint16_t player_target;
    sf::Vector2f attack_vector{};

    std::map<uint16_t, sf::Clock> invulnerability_timers;
    sf::Clock despawn_timer;

    sf::Vector2f knockback_vector{};
    float knockback_distance;
    float knockback_duration;

    enum class AttackState
    {
        Start, Windup, Hit, Followthrough, Cooldown
    };

    AttackState attack_state = AttackState::Start;
    sf::Vector2f starting_attack_position{};
    sf::Clock attack_timer;

    enum class KnockbackState
    {
        Start, Knockback
    };

    KnockbackState knockback_state = KnockbackState::Start;
    sf::Clock knockback_timer;

    bool stunned = false;
    sf::Clock stun_timer;

    void setBehavior(Behavior behavior, std::vector<PlayerInfo>& players);
    void setAction(Action action, std::vector<PlayerInfo>& players);
    void resetActionStates();
    void chooseAction(sf::Time elapsed, shared::ConvoyDefinition convoy, std::vector<PlayerInfo>& players, std::vector<sf::FloatRect> obstacles);
    void checkAggro(std::vector<PlayerInfo>& players);
    sf::Vector2f getTargetConvoyPoint(shared::ConvoyDefinition convoy);
    sf::Vector2f getTargetPlayerPoint(std::vector<PlayerInfo>& players);
    void move(sf::Time elapsed, sf::Vector2f destination, std::vector<sf::FloatRect> obstacles);
    void handleAttack(sf::Time elapsed, std::vector<PlayerInfo>& players);
    void checkAttackHit(std::vector<PlayerInfo>& players);
    void handleKnockback(sf::Time elapsed, std::vector<PlayerInfo>& players, shared::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles);
    void handleStunned(std::vector<PlayerInfo>& players);
};

} // server
