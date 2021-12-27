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
#include "region_definitions.h"
#include "messaging.h"

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
        Sniffing,
        Stunned,
        Leaping
    };

    Enemy();

    void Update(sf::Time elapsed, definitions::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles);
    void WeaponHit(uint16_t player_id, uint8_t damage, definitions::WeaponKnockback knockback, sf::Vector2f hit_vector);

    Behavior GetBehavior();
    Action GetAction();
    sf::FloatRect GetBounds();
    int GetSiphonRate();

    network::EnemyData Data{};
    bool Despawn = false;

private:
    definitions::EntityDefinition definition;

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

    float sniff_cooldown;
    sf::Clock sniff_timer;

    bool stunned = false;
    sf::Clock stun_timer;

    enum class LeapingState
    {
        Start, Windup, Leap, Cleanup
    };

    LeapingState leaping_state = LeapingState::Start;
    sf::Vector2f leap_vector;
    float leap_cooldown;
    sf::Clock leap_timer;

    void setActionFlags();
    void setBehavior(Behavior behavior);
    void setAction(Action action);
    void resetActionStates();
    void chooseAction(sf::Time elapsed, definitions::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles);
    void checkAggro();
    sf::Vector2f getTargetConvoyPoint(definitions::ConvoyDefinition convoy);
    sf::Vector2f getTargetPlayerPoint();
    void move(sf::Time elapsed, sf::Vector2f destination, std::vector<sf::FloatRect> obstacles);
    void handleAttack(sf::Time elapsed);
    bool checkAttackHit();
    bool checkLeapHit();
    void handleKnockback(sf::Time elapsed, std::vector<sf::FloatRect> obstacles);
    void handleStunned();
    void handleSniffing();
    void handleLeaping(sf::Time elapsed, std::vector<sf::FloatRect> obstacles);
};

} // server
