/**************************************************************************************************
 *  File:       enemy.cpp
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "enemy.h"
#include "pathfinding.h"
#include "messaging.h"
#include "global_state.h"
#include <iostream>
#include <random>
#include <SFML/System/Sleep.hpp>

using std::cout, std::endl;
using server::global::PlayerList;

namespace server {

namespace {
    constexpr int BASE_AGGRO_RADIUS = 175; // pixels
    constexpr float AGGRO_COEFFICIENT = 0.17;
    constexpr float CHARGE_RATE = 10;
    constexpr float STUN_DURATION = 0.3; // seconds
    constexpr int BASE_SIPHON_RATE = 3;
    constexpr int DESPAWN_TIME = 5; // seconds
    constexpr int SNIFF_COOLDOWN = 4; // seconds
    constexpr float SNIFF_DURATION = 1.5;
    constexpr float LEAP_COOLDOWN = 4;
    constexpr float LEAP_WINDUP = 0.75f; // seconds
}

Enemy::Enemy() : Enemy(true) { }

Enemy::Enemy(bool will_attack_convoy) : attack_convoy{will_attack_convoy}
{
    static uint16_t identifier = 0;
    Data.id = identifier++;
    Data.health = 100;
    Data.type = definitions::EntityType::SmallDemon;

    if (attack_convoy)
    {
        current_behavior = Behavior::Moving;
    }
    else
    {
        current_behavior = Behavior::Idle;
    }

    current_action  = Action::None;

    definition = definitions::GetEntityDefinition(Data.type);

    sniff_cooldown = SNIFF_COOLDOWN + util::GetRandomInt(0, 1);
    leap_cooldown = 0;

    resetActionStates();
}

sf::FloatRect Enemy::GetBounds()
{
    sf::Vector2f size = definition.size * (1 + Data.charge / 100.0f);
    return sf::FloatRect(Data.position.x - size.x / 2, Data.position.y - size.y / 2, size.x, size.y);
}

int Enemy::GetSiphonRate()
{
    return BASE_SIPHON_RATE;
}

void Enemy::Update(sf::Time elapsed, definitions::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles)
{
    updateTimers(elapsed);

    switch (current_action)
    {
        case Action::Attacking:
        {
            handleAttack(elapsed);
        }
        break;
        case Action::Knockback:
        {
            handleKnockback(elapsed, obstacles);
        }
        break;
        case Action::Stunned:
        {
            handleStunned();
        }
        break;
        case Action::Sniffing:
        {
            handleSniffing();
        }
        break;
        case Action::Leaping:
        {
            handleLeaping(elapsed, obstacles);
        }
        break;
        case Action::None:
        {
            chooseAction(elapsed, convoy, obstacles);
        }
        break;
    }
}

void Enemy::WeaponHit(uint16_t player_id, uint8_t damage, definitions::WeaponKnockback knockback, sf::Vector2f hit_vector, float invulnerability_window)
{
    if (invulnerability_timers.find(player_id) == invulnerability_timers.end())
    {
        invulnerability_timers[player_id] = 0;
    }
    else if (invulnerability_timers[player_id] < invulnerability_windows[player_id])
    {
        return;
    }
    else if (Data.health == 0)
    {
        return;
    }

    invulnerability_timers[player_id] = 0;
    invulnerability_windows[player_id] = invulnerability_window;

    if (Data.health < damage)
    {
        Data.health = 0;
    }
    else
    {
        Data.health -= damage;
    }

    if (Data.health == 0)
    {
        setAction(Action::None);
        setBehavior(Behavior::Dead);
    }

    if (knockback.distance > 0)
    {
        knockback_vector = util::Normalize(hit_vector) * (knockback.distance / knockback.duration);
        knockback_distance = knockback.distance;
        knockback_duration = knockback.duration;

        setAction(Action::Knockback);
    }
}

Enemy::Behavior Enemy::GetBehavior()
{
    return current_behavior;
}

Enemy::Action Enemy::GetAction()
{
    return current_action;
}

void Enemy::updateTimers(sf::Time elapsed)
{
    for (auto& [id, timer] : invulnerability_timers)
    {
        timer += elapsed.asSeconds();
    }

    despawn_timer += elapsed.asSeconds();
    attack_timer += elapsed.asSeconds();
    knockback_timer += elapsed.asSeconds();
    sniff_timer += elapsed.asSeconds();
    stun_timer += elapsed.asSeconds();
    leap_timer += elapsed.asSeconds();
}

void Enemy::setActionFlags()
{
    network::EnemyAction enemy_action{};

    switch (current_action)
    {
        case Action::Attacking:
        {
            for (auto& player : PlayerList)
            {
                if (player.Data.id == player_target)
                {
                    attack_vector = util::Normalize(player.Data.position - Data.position);
                    break;
                }
            }

            enemy_action.flags.start_attack = true;
            enemy_action.attack_vector = attack_vector;
        }
        break;
        case Action::Knockback:
        {
            enemy_action.flags.knockback = true;
        }
        break;
        case Action::Stunned:
        {
            enemy_action.flags.stunned = true;
        }
        break;
        case Action::Sniffing:
        {
            enemy_action.flags.sniffing = true;
        }
        break;
        case Action::Leaping:
        {
            for (auto& player : PlayerList)
            {
                if (player.Data.id == player_target)
                {
                    leap_vector = util::Normalize(player.Data.position - Data.position);
                    break;
                }
            }

            enemy_action.flags.leaping = true;
        }
        break;
        case Action::None:
        {
            switch (current_behavior)
            {
                case Behavior::Idle:
                case Behavior::Moving:
                case Behavior::Hunting:
                {
                    enemy_action.flags.move = true;
                }
                break;
                case Behavior::Feeding:
                {
                    enemy_action.flags.feed = true;
                }
                break;
                case Behavior::Dead:
                {
                    enemy_action.flags.dead = true;
                    despawn_timer = 0;
                }
                break;
            }
        }
        break;
    }

    for (auto& player : PlayerList)
    {
        network::ServerMessage::EnemyChangeAction(*player.Socket, Data.id, enemy_action);
    }
}

void Enemy::setBehavior(Behavior behavior)
{
    current_behavior = behavior;
    setActionFlags();
}

void Enemy::setAction(Action action)
{
    resetActionStates();
    current_action = action;
    setActionFlags();
}

void Enemy::resetActionStates()
{
    attack_state = AttackState::Start;
    knockback_state = KnockbackState::Start;
    stunned = false;
    leaping_state = LeapingState::Start;
}

void Enemy::chooseAction(sf::Time elapsed, definitions::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles)
{
    checkAggro();
    sf::Vector2f destination;

    switch (current_behavior)
    {
        case Behavior::Idle:
        {
            if (sniff_timer > sniff_cooldown)
            {
                setAction(Action::Sniffing);
            }

            destination = Data.position;
        }
        break;
        case Behavior::Moving:
        {
            destination = getTargetConvoyPoint(convoy);
            if (util::Distance(Data.position, destination) < definition.feeding_range)
            {
                setBehavior(Behavior::Feeding);
                return;
            }

            if (sniff_timer > sniff_cooldown)
            {
                setAction(Action::Sniffing);
            }
        }
        break;
        case Behavior::Feeding:
        {
            Data.charge += CHARGE_RATE * elapsed.asSeconds();
            if (Data.charge > 100)
            {
                Data.charge = 100;
            }
            return;
        }
        break;
        case Behavior::Hunting:
        {
            destination = getTargetPlayerPoint();
            double target_distance = util::Distance(Data.position, destination);
            if (target_distance < definition.attack_range)
            {
                setAction(Action::Attacking);
                return;
            }
            else if (target_distance > definition.minimum_leap_range && target_distance < definition.maximum_leap_range && leap_timer >= leap_cooldown)
            {
                if (util::GetRandomInt(0, 5) == 0)
                {
                    setAction(Action::Leaping);
                }
                else
                {
                    leap_cooldown = 0.25f;
                    leap_timer = 0;
                }
            }
        }
        break;
        case Behavior::Dead:
        {
            if (!Despawn)
            {
                Despawn = true;
            }
            return;
        }
        break;
    }

    move(elapsed, destination, obstacles);
}

void Enemy::checkAggro()
{
    // Aggro radius on PlayerList goes up polynomially as charge goes up
    float aggro_range = AGGRO_COEFFICIENT * (Data.charge * Data.charge) + BASE_AGGRO_RADIUS;

    switch (current_behavior)
    {
        case Behavior::Hunting:
        {
            // TODO: Change targets when another player is much closer?
            for (auto& player : PlayerList)
            {
                if (player_target == player.Data.id && player.Status != Player::PlayerStatus::Alive)
                {
                    if (attack_convoy)
                    {
                        setBehavior(Behavior::Moving);
                    }
                    else
                    {
                        setBehavior(Behavior::Idle);
                    }
                    break;
                }
            }
            return;
        }
        break;
        case Behavior::Feeding:
        {
            aggro_range -= 200;
            [[fallthrough]];
        }
        case Behavior::Idle:
        {
            [[fallthrough]];
        }
        case Behavior::Moving:
        {
            float target_distance = aggro_range + 1;

            for (auto& player : PlayerList)
            {
                if (player.Status == Player::PlayerStatus::Alive)
                {
                    float player_distance = util::Distance(player.Data.position, Data.position);
                    if (player_distance < target_distance)
                    {
                        target_distance = player_distance;
                        player_target = player.Data.id;
                        setBehavior(Behavior::Hunting);
                    }
                }
            }
        }
        break;
        case Behavior::Dead: { }
        break;
    }
}

sf::Vector2f Enemy::getTargetConvoyPoint(definitions::ConvoyDefinition convoy)
{
    sf::Vector2f destination;
    sf::FloatRect convoy_bounds = convoy.GetBounds();

    if (Data.position.x >= convoy_bounds.left && Data.position.x <= convoy_bounds.left + convoy_bounds.width)
    {
        if (Data.position.y <= convoy_bounds.top)
        {
            destination.x = Data.position.x;
            destination.y = convoy_bounds.top - 1;
        }
        else
        {
            destination.x = Data.position.x;
            destination.y = convoy_bounds.top + convoy_bounds.height + 1;
        }
    }
    else if (Data.position.y >= convoy_bounds.top && Data.position.y <= convoy_bounds.top + convoy_bounds.height)
    {
        if (Data.position.x <= convoy_bounds.left)
        {
            destination.x = convoy_bounds.left - 1;
            destination.y = Data.position.y;
        }
        else
        {
            destination.x = convoy_bounds.left + convoy_bounds.width + 1;
            destination.y = Data.position.y;
        }
    }
    else
    {
        if (Data.position.x <= convoy_bounds.left && Data.position.y <= convoy_bounds.top) // upper-left
        {
            destination.x = convoy_bounds.left - 1;
            destination.y = convoy_bounds.top - 1;
        }
        else if (Data.position.x <= convoy_bounds.left && Data.position.y >= convoy_bounds.top + convoy_bounds.height) // lower-left
        {
            destination.x = convoy_bounds.left - 1;
            destination.y = convoy_bounds.top + convoy_bounds.height + 1;
        }
        else if (Data.position.x >= convoy_bounds.left + convoy_bounds.width && Data.position.y <= convoy_bounds.top) // upper-right
        {
            destination.x = convoy_bounds.left + convoy_bounds.width + 1;
            destination.y = convoy_bounds.top - 1;
        }
        else if (Data.position.x >= convoy_bounds.left + convoy_bounds.width && Data.position.y >= convoy_bounds.top + convoy_bounds.height) // lower-right
        {
            destination.x = convoy_bounds.left + convoy_bounds.width + 1;
            destination.y = convoy_bounds.top + convoy_bounds.height + 1;
        }
    }

    return destination;
}

sf::Vector2f Enemy::getTargetPlayerPoint()
{
    for (auto& player : PlayerList)
    {
        if (player.Data.id == player_target)
        {
            // TODO: Calculate a more specific destination so as not to get errors finding a path
            return player.Data.position;
        }
    }

    if (attack_convoy)
    {
        setBehavior(Behavior::Moving);
    }
    else
    {
        setBehavior(Behavior::Idle);
    }

    return Data.position;
}

void Enemy::move(sf::Time elapsed, sf::Vector2f destination, std::vector<sf::FloatRect> obstacles)
{
    util::PathingGraph graph = util::CreatePathingGraph(Data.position, destination, obstacles, GetBounds().getSize());
    std::list<sf::Vector2f> path = util::GetPath(graph);
    sf::Vector2f target_point = path.front();

    if (target_point != Data.position)
    {
        sf::Vector2f movement_vector = target_point - Data.position;
        sf::Vector2f velocity = util::Normalize(movement_vector) * (definition.base_movement_speed * (1 + Data.charge / 100));

        Data.position += velocity * elapsed.asSeconds();
    }
}

void Enemy::handleAttack(sf::Time elapsed)
{
    sf::Vector2f velocity;

    switch (attack_state)
    {
        case AttackState::Start:
        {
            starting_attack_position = Data.position;
            attack_state = AttackState::Windup;
            attack_timer = 0;
            [[fallthrough]];
        }
        case AttackState::Windup:
        {
            velocity = attack_vector * (definition.attack_distance / definition.attack_duration);
            Data.position += velocity * elapsed.asSeconds();
            if (attack_timer >= definition.attack_duration / 2)
            {
                attack_state = AttackState::Hit;
            }
        }
        break;
        case AttackState::Hit:
        {
            checkAttackHit();
            attack_state = AttackState::Followthrough;
            [[fallthrough]];
        }
        case AttackState::Followthrough:
        {
            velocity = -attack_vector * (definition.attack_distance / definition.attack_duration);
            Data.position += velocity * elapsed.asSeconds();
            if (attack_timer >= definition.attack_duration)
            {
                attack_state = AttackState::Cooldown;
                Data.position = starting_attack_position;
            }
        }
        break;
        case AttackState::Cooldown:
        {
            if (attack_timer >= (definition.attack_duration + definition.attack_cooldown))
            {
                setAction(Action::None);
                attack_state = AttackState::Start;
            }
        }
        break;
    }
}

bool Enemy::checkAttackHit()
{
    bool hit = false;
    for (auto& player : PlayerList)
    {
        if (player.Status == Player::PlayerStatus::Alive)
        {
            sf::FloatRect bounds(player.Data.position.x - 35, player.Data.position.y - 35, 70, 70);
            if (util::Intersects(bounds, GetBounds()))
            {
                player.Damage(definition.attack_damage);
                hit = true;
            }
        }
    }

    return hit;
}

bool Enemy::checkLeapHit()
{
    bool hit = false;
    for (auto& player : PlayerList)
    {
        if (player.Status == Player::PlayerStatus::Alive)
        {
            if (util::Intersects(player.GetBounds(), GetBounds()))
            {
                player.Damage(definition.attack_damage);
                hit = true;
            }
        }
    }

    return hit;
}

void Enemy::handleKnockback(sf::Time elapsed, std::vector<sf::FloatRect> obstacles)
{
    switch (knockback_state)
    {
        case KnockbackState::Start:
        {
            knockback_timer = 0;
            knockback_state = KnockbackState::Knockback;
            [[fallthrough]];
        };
        case KnockbackState::Knockback:
        {
            sf::Vector2f new_position = Data.position + knockback_vector * elapsed.asSeconds();
            sf::FloatRect new_bounds = GetBounds();
            new_bounds.left = new_position.x;
            new_bounds.top = new_position.y;

            bool collision = false;
            for (auto& obstacle : obstacles)
            {
                if (util::Intersects(new_bounds, obstacle))
                {
                    collision = true;
                }
            }

            if (!collision)
            {
                Data.position = new_position;
            }
            if (collision || knockback_timer >= knockback_duration)
            {
                knockback_state = KnockbackState::Start;
                setAction(Action::Stunned);
            }
        }
        break;
    }
}

void Enemy::handleStunned()
{
    if (!stunned)
    {
        stun_timer = 0;
        stunned = true;
    }

    if (stun_timer >= STUN_DURATION)
    {
        setAction(Action::None);
        stunned = false;
    }
}

void Enemy::handleSniffing()
{
    if (sniff_timer > sniff_cooldown + SNIFF_DURATION)
    {
        sniff_cooldown = SNIFF_COOLDOWN + util::GetRandomInt(0, 2);
        sniff_timer = 0;
        setAction(Action::None);
    }
}

void Enemy::handleLeaping(sf::Time elapsed, std::vector<sf::FloatRect> obstacles)
{
    static util::Seconds leap_tracking_timer;
    leap_tracking_timer += elapsed.asSeconds();

    switch (leaping_state)
    {
        case LeapingState::Start:
        {
            leap_tracking_timer = 0;
            leaping_state = LeapingState::Windup;
            [[fallthrough]];
        }
        case LeapingState::Windup:
        {
            if (leap_tracking_timer > LEAP_WINDUP)
            {
                leaping_state = LeapingState::Leap;
            }
        }
        break;
        case LeapingState::Leap:
        {
            if (leap_tracking_timer > LEAP_WINDUP + (definition.leaping_distance / definition.leaping_speed))
            {
                setAction(Action::None);
                leaping_state = LeapingState::Cleanup;
            }

            sf::Vector2f velocity = leap_vector * definition.leaping_speed * elapsed.asSeconds();

            sf::Vector2f new_position = Data.position + velocity;
            sf::FloatRect new_bounds = GetBounds();
            new_bounds.left = new_position.x;
            new_bounds.top = new_position.y;

            bool collision = false;
            for (auto& obstacle : obstacles)
            {
                if (util::Intersects(new_bounds, obstacle))
                {
                    collision = true;
                }
            }

            if (checkLeapHit())
            {
                collision = true;
            }

            if (collision)
            {
                leaping_state = LeapingState::Cleanup;
                setAction(Action::Stunned);
            }
            else
            {
                Data.position = new_position;
            }
        }
        break;
        case LeapingState::Cleanup:
        {
            leap_timer = 0;
            leap_cooldown = LEAP_COOLDOWN;
            leaping_state = LeapingState::Start;
        }
        break;
    }
}

} // server
