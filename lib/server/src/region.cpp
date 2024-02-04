/**************************************************************************************************
 *  File:       region.cpp
 *  Class:      Region
 *
 *  Purpose:    A single instanced region at a node
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "region.h"
#include "definitions.h"
#include "game_math.h"
#include "global_state.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

using network::ClientMessage, network::ServerMessage;
using server::global::PlayerList;

using std::cout, std::cerr, std::endl;

namespace server {

Region::Region() { }

Region::Region(definitions::RegionType region_type, int player_count, float battery_level) : BatteryLevel{battery_level}, num_players{player_count}
{
    definition = definitions::GetRegionDefinition(region_type);

    Bounds = definition.bounds;
    Convoy = definition.convoy;

    for (auto& obstacle : definition.obstacles)
    {
        Obstacles.push_back(obstacle.bounds);
    }

    auto convoy_collisions = Convoy.GetCollisions();
    Obstacles.insert(Obstacles.end(), convoy_collisions.begin(), convoy_collisions.end());

    Leyline = definition.leyline;

    if (definition.leyline)
    {
        battery_charge_rate = 5;
    }

    if (region_type == definitions::RegionType::MenuEvent)
    {
        global::Paused = true;
        global::MenuEvent = true;
        current_event = definitions::GetNextMenuEvent();
        for (auto& p : PlayerList)
        {
            ServerMessage::SetMenuEvent(*p.Socket, current_event.event_id);
        }
    }

    for (auto& pack : definition.enemy_packs)
    {
        spawnPack(pack);
    }
}

void Region::Update(sf::Time elapsed)
{
    if (global::Paused)
    {
        return;
    }

    region_age += elapsed.asSeconds();

    for (auto& enemy : Enemies)
    {
        enemy.Update(elapsed);
    }

    handleProjectiles(elapsed);
    updateBattery(elapsed);

    if (definition.leyline)
    {
        spawnWave(elapsed);
    }
}

namespace {

struct WinningLink
{
    uint16_t value;
    bool finish;
};

WinningLink getWinnerValue(definitions::MenuEventOption option)
{
    float weight_sum = 0;
    for (auto& link : option.links)
    {
        weight_sum += link.weight;
    }

    float roll = util::GetRandomFloat(0, weight_sum);

    weight_sum = 0;
    for (auto& link : option.links)
    {
        if (roll >= weight_sum && roll < weight_sum + link.weight)
        {
            return {link.value, link.finish};
        }

        weight_sum += link.weight;
    }

    throw (std::runtime_error("Winner link not found?"));
}

} // Anonymous namespace

bool Region::AdvanceMenuEvent(uint16_t winner, uint16_t& out_event_id, uint16_t& out_event_action)
{
    if (winner > current_event.pages[current_event.current_page].options.size())
    {
        cerr << "Menu event tried to advance to an option that does not exist." << endl;
        return false;
    }

    WinningLink winning_link = getWinnerValue(current_event.pages[current_event.current_page].options[winner]);

    if (winning_link.value > current_event.pages.size() && !winning_link.finish)
    {
        cerr << "Winning vote does not link to a valid page." << endl;
        return false;
    }

    for (auto& player : PlayerList)
    {
        ServerMessage::AdvanceMenuEvent(*player.Socket, winning_link.value, winning_link.finish);
    }

    if (!winning_link.finish)
    {
        current_event.current_page = winning_link.value;
    }
    else
    {
        global::Paused = false;
        global::MenuEvent = false;
        out_event_id = current_event.event_id;
        out_event_action = winning_link.value;
        return true;
    }

    return false;
}

void Region::updateBattery(sf::Time elapsed)
{
    int siphon_rate = 0;
    for (auto& enemy : Enemies)
    {
        if (enemy.GetBehavior() == definitions::Behavior::Feeding)
        {
            siphon_rate += enemy.GetSiphonRate();
        }
    }

    if (region_age < 6)
    {
        BatteryLevel -= siphon_rate * elapsed.asSeconds();
    }
    else
    {
        BatteryLevel += (battery_charge_rate - siphon_rate) * elapsed.asSeconds();
    }

    if (BatteryLevel < 0)
    {
        BatteryLevel = 0;
    }

    if (BatteryLevel > 1000)
    {
        BatteryLevel = 1000;
    }
}

bool Region::spawnWave(sf::Time elapsed)
{
    constexpr util::Seconds SPAWN_INTERVAL = 15;
    age_timer += elapsed.asSeconds();

    if (age_timer < SPAWN_INTERVAL)
    {
        return false;
    }

    age_timer = 0;

    for (int i = 0; i < num_players; ++i)
    {
        // TODO: Vary pack difficulty for partial region difficulties; like difficulty 1.4 should be a 60% chance for difficulty 1 and 40% for difficulty 2
        definitions::EnemyPack pack = definitions::GetEnemyPackByDifficulty(region_difficulty);
        pack.position.x = util::GetRandomFloat(definition.spawn_zone.left, definition.spawn_zone.left + definition.spawn_zone.width);
        pack.position.y = util::GetRandomFloat(definition.spawn_zone.top, definition.spawn_zone.top + definition.spawn_zone.height);

        spawnPack(pack);
    }

    return true;
}

void Region::spawnEnemy(definitions::EntityType type, sf::Vector2f position)
{
    spawnEnemy(type, position, position);
}

void Region::spawnPack(definitions::EnemyPack pack)
{
    for (auto& spawn : pack.spawns)
    {
        int count = util::GetRandomInt(spawn.min, spawn.max);

        for (int i = 0; i < count; ++i)
        {
            spawnEnemy(spawn.type, util::GetRandomPositionFromPoint(pack.position, 0, 60), pack.position);
        }
    }
}

void Region::spawnEnemy(definitions::EntityType type, sf::Vector2f position, sf::Vector2f pack_position)
{
    Enemy enemy(this, type, position, pack_position);
    Enemies.push_back(enemy);
    for (auto& player : PlayerList)
    {
        ServerMessage::AddEnemy(*player.Socket, enemy.GetData().id, type);
    }

    if (PathingGraphs.find(type) == PathingGraphs.end())
    {
        PathingGraphs[type] = util::CreatePathingGraph(Obstacles, enemy.GetPathingSize());
    }
}

void Region::handleProjectiles(sf::Time elapsed)
{
    auto iterator = Projectiles.begin();
    while (iterator != Projectiles.end())
    {
        auto& projectile = *iterator;
        projectile.position += projectile.velocity * elapsed.asSeconds();
        bool destroy = false;
        if (projectile.hostile == false)
        {
            for (auto& enemy : Enemies)
            {
                if (util::Contains(enemy.GetBounds(), projectile.position))
                {
                    enemy.WeaponHit(projectile.owner, projectile.damage, projectile.knockback, -projectile.velocity, projectile.invulnerability_window);
                    destroy = true;
                }
            }
        }

        for (auto& obstacle : Obstacles)
        {
            if (util::Contains(obstacle, projectile.position))
            {
                destroy = true;
            }
        }

        if (destroy)
        {
            iterator = Projectiles.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

} // namespace server
