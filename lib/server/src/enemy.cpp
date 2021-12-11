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
#include <iostream>
#include <SFML/System/Sleep.hpp>

using std::cout, std::endl;

namespace server {

namespace {
    constexpr int BASE_AGGRO_RADIUS = 350; // pixels
    constexpr float AGGRO_COEFFICIENT = 0.17;
    constexpr float CHARGE_RATE = 10;
    constexpr float INVULNERABILITY_WINDOW = 0.3; // seconds
    constexpr float STUN_DURATION = 0.3; // seconds
    constexpr int BASE_SIPHON_RATE = 3;
    constexpr int DESPAWN_TIME = 5; // seconds
}

Enemy::Enemy()
{
    static uint16_t identifier = 0;
    Data.id = identifier++;
    Data.health = 100;
    Data.type = network::EnemyData::EnemyType::SmallDemon;
    current_behavior = Behavior::Moving;
    current_action  = Action::None;
    resetActionStates();
}

sf::FloatRect Enemy::GetBounds()
{
    return sf::FloatRect(Data.position.x - 25, Data.position.y - 25, base_size.x * (1 + Data.charge / 100.0), base_size.y * (1 + Data.charge / 100.0));
}

int Enemy::GetSiphonRate()
{
    return BASE_SIPHON_RATE;
}

void Enemy::Update(sf::Time elapsed, shared::ConvoyDefinition convoy, std::vector<PlayerInfo>& players, std::vector<sf::FloatRect> obstacles)
{
    switch (current_action)
    {
        case Action::Attacking:
        {
            handleAttack(elapsed, players);
        }
        break;
        case Action::Knockback:
        {
            handleKnockback(elapsed, players, convoy, obstacles);
        }
        break;
        case Action::Stunned:
        {
            handleStunned(players);
        }
        break;
        case Action::None:
        {
            chooseAction(elapsed, convoy, players, obstacles);
        }
        break;
    }
}

void Enemy::WeaponHit(uint16_t player_id, uint8_t damage, PlayerInfo::WeaponKnockback knockback, sf::Vector2f hit_vector, std::vector<PlayerInfo>& players)
{
    if (invulnerability_timers.find(player_id) == invulnerability_timers.end())
    {
        invulnerability_timers[player_id] = sf::Clock();
    }
    else if (invulnerability_timers[player_id].getElapsedTime().asSeconds() < INVULNERABILITY_WINDOW)
    {
        return;
    }
    else if (Data.health == 0)
    {
        return;
    }

    invulnerability_timers[player_id].restart();

    if (Data.health < damage)
    {
        Data.health = 0;
    }
    else
    {
        Data.health -= damage;
    }

    if (knockback.distance > 0)
    {
        knockback_vector = util::Normalize(hit_vector) * (knockback.distance / knockback.duration);
        knockback_distance = knockback.distance;
        knockback_duration = knockback.duration;

        setAction(Action::Knockback, players);
    }

    if (Data.health == 0)
    {
        setBehavior(Behavior::Dead, players);
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


void Enemy::setBehavior(Behavior behavior, std::vector<PlayerInfo>& players)
{
    current_behavior = behavior;
    network::EnemyAction enemy_action{};
    switch (behavior)
    {
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
            despawn_timer.restart();
        }
        break;
    }

    for (auto& player : players)
    {
        network::ServerMessage::EnemyChangeAction(*player.Socket, Data.id, enemy_action);
    }
}

void Enemy::setAction(Action action, std::vector<PlayerInfo>& players)
{
    resetActionStates();
    current_action = action;
    network::EnemyAction enemy_action{};
    switch (action)
    {
        case Action::Attacking:
        {
            for (auto& player : players)
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
        default: { }
    }

    for (auto& player : players)
    {
        network::ServerMessage::EnemyChangeAction(*player.Socket, Data.id, enemy_action);
    }
}

void Enemy::resetActionStates()
{
    attack_state = AttackState::Start;
    knockback_state = KnockbackState::Start;
    stunned = false;
}

void Enemy::chooseAction(sf::Time elapsed, shared::ConvoyDefinition convoy, std::vector<PlayerInfo>& players, std::vector<sf::FloatRect> obstacles)
{
    checkAggro(players);
    sf::Vector2f destination;

    switch (current_behavior)
    {
        case Behavior::Moving:
        {
            destination = getTargetConvoyPoint(convoy);
            if (util::Distance(Data.position, destination) < feeding_range)
            {
                setBehavior(Behavior::Feeding, players);
                return;
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
            destination = getTargetPlayerPoint(players);
            if (util::Distance(Data.position, destination) < attack_range)
            {
                setAction(Action::Attacking, players);
                return;
            }
        }
        break;
        case Behavior::Dead:
        {
            if (!Despawn /* && despawn_timer.getElapsedTime().asSeconds() > DESPAWN_TIME*/)
            {
                Despawn = true;
            }
            return;
        }
        break;
    }

    move(elapsed, destination, obstacles);
}

void Enemy::checkAggro(std::vector<PlayerInfo>& players)
{
    // Aggro radius on players goes up polynomially as charge goes up
    float aggro_range = AGGRO_COEFFICIENT * (Data.charge * Data.charge) + BASE_AGGRO_RADIUS;

    switch (current_behavior)
    {
        case Behavior::Hunting:
        {
            // TODO: Change targets when another player is much closer?
            for (auto& player : players)
            {
                if (player_target == player.Data.id && player.Status != PlayerInfo::PlayerStatus::Alive)
                {
                    setBehavior(Behavior::Moving, players);
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
        case Behavior::Moving:
        {
            float target_distance = aggro_range + 1;

            for (auto& player : players)
            {
                if (player.Status == PlayerInfo::PlayerStatus::Alive)
                {
                    float player_distance = util::Distance(player.Data.position, Data.position);
                    if (player_distance < target_distance)
                    {
                        target_distance = player_distance;
                        player_target = player.Data.id;
                        setBehavior(Behavior::Hunting, players);
                    }
                }
            }
        }
        break;
        case Behavior::Dead: { }
        break;
    }
}

sf::Vector2f Enemy::getTargetConvoyPoint(shared::ConvoyDefinition convoy)
{
    sf::Vector2f destination;
    sf::FloatRect convoy_bounds = shared::GetConvoyBounds(convoy);

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

sf::Vector2f Enemy::getTargetPlayerPoint(std::vector<PlayerInfo>& players)
{
    for (auto& player : players)
    {
        if (player.Data.id == player_target)
        {
            // TODO: Calculate a more specific destination so as not to get errors finding a path
            return player.Data.position;
        }
    }

    setBehavior(Behavior::Moving, players);
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
        sf::Vector2f velocity = util::Normalize(movement_vector) * (movement_speed * (1 + Data.charge / 100));

        Data.position += velocity * elapsed.asSeconds();
    }
}

void Enemy::handleAttack(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    sf::Vector2f velocity;

    switch (attack_state)
    {
        case AttackState::Start:
        {
            starting_attack_position = Data.position;
            attack_state = AttackState::Windup;
            attack_timer.restart();
            [[fallthrough]];
        }
        case AttackState::Windup:
        {
            velocity = attack_vector * (attack_distance / attack_duration);
            Data.position += velocity * elapsed.asSeconds();
            if (attack_timer.getElapsedTime().asSeconds() >= attack_duration / 2)
            {
                attack_state = AttackState::Hit;
            }
        }
        break;
        case AttackState::Hit:
        {
            checkAttackHit(players);
            attack_state = AttackState::Followthrough;
            [[fallthrough]];
        }
        case AttackState::Followthrough:
        {
            velocity = -attack_vector * (attack_distance / attack_duration);
            Data.position += velocity * elapsed.asSeconds();
            if (attack_timer.getElapsedTime().asSeconds() >= attack_duration)
            {
                attack_state = AttackState::Cooldown;
                Data.position = starting_attack_position;
            }
        }
        break;
        case AttackState::Cooldown:
        {
            if (attack_timer.getElapsedTime().asSeconds() >= (attack_duration + attack_cooldown))
            {
                setAction(Action::None, players);
                attack_state = AttackState::Start;
            }
        }
        break;
    }
}

void Enemy::checkAttackHit(std::vector<PlayerInfo>& players)
{
    for (auto& player : players)
    {
        if (player.Status == PlayerInfo::PlayerStatus::Alive)
        {
            sf::FloatRect bounds(player.Data.position.x - 35, player.Data.position.y - 35, 70, 70);
            if (bounds.intersects(GetBounds()))
            {
                player.Damage(attack_damage);
            }
        }
    }
}

void Enemy::handleKnockback(sf::Time elapsed, std::vector<PlayerInfo>& players, shared::ConvoyDefinition convoy, std::vector<sf::FloatRect> obstacles)
{
    switch (knockback_state)
    {
        case KnockbackState::Start:
        {
            knockback_timer.restart();
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

            sf::FloatRect convoy_bounds = shared::GetConvoyBounds(convoy);
            if (util::Intersects(new_bounds, convoy_bounds))
            {
                collision = true;
            }

            if (!collision)
            {
                Data.position = new_position;
            }
            if (collision || knockback_timer.getElapsedTime().asSeconds() >= knockback_duration)
            {
                knockback_state = KnockbackState::Start;
                setAction(Action::Stunned, players);
            }
        }
        break;
    }
}

void Enemy::handleStunned(std::vector<PlayerInfo>& players)
{
    if (!stunned)
    {
        stun_timer.restart();
        stunned = true;
    }

    if (stun_timer.getElapsedTime().asSeconds() >= STUN_DURATION)
    {
        setAction(Action::None, players);
        stunned = false;
    }
}

} // server
