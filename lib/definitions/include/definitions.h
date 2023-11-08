/**************************************************************************************************
 *  File:       region_definitions.h
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <map>
#include <vector>
#include <string>
#include <optional>
#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "game_math.h"

namespace definitions
{

enum class EntityType : uint8_t
{
    Player,
    SmallDemon,
    Bat
};

//struct EntityDefinition
//{
//    sf::Vector2f size;
//    float base_movement_speed;
//    float attack_damage;
//    float attack_range;
//    float minimum_leap_range;
//    float maximum_leap_range;
//    float leaping_speed;
//    float leaping_distance;
//    float feeding_range;
//    float attack_distance;
//    float attack_duration;
//    float attack_cooldown;
//};

enum class Behavior
{
    None,
    Wandering,
    Feeding,
    Hunting,
    Stalking,
    Dead
};

enum class Action
{
    None,
    Tackling,
    Knockback,
    Sniffing,
    Stunned,
    Leaping
};

struct AttackDefinition
{
    float range;
    float damage;
    util::Seconds cooldown;
    util::Seconds cooldown_timer;
    float knockback_distance;
};

struct AttackEvent
{
    AttackDefinition attack;
    sf::Vector2f origin;
};

struct EntityDefinition
{
    //std::string name;
    float base_movement_speed;
    float walking_speed;
    int feeding_range;
    float siphon_rate;
    std::map<Behavior, bool> behaviors;
    std::map<Action, bool> actions;
    std::map<Action, std::optional<AttackDefinition>> attacks;
    sf::Vector2f hitbox;
    int steering_force;
    int repulsion_force;
    int repulsion_radius;
    int acceleration;
    int deceleration;
    float wander_rest_time_min;
    float wander_rest_time_max;
    int aggro_range;
    int close_quarters_range;
    int leash_range;
    float base_aggression;
    util::Seconds leap_windup_time;
    util::Seconds leap_time;
    util::Seconds leap_rest_time;
};

EntityDefinition GetEntityDefinition(EntityType type);

enum class WeaponType
{
    Sword, BurstGun, HitscanGun
};

struct WeaponKnockback
{
    float distance; // total distance over duration in pixels
    util::Seconds duration; // duration in seconds
    util::Seconds stun_duration; // Stun after knockback is over
};

struct Weapon
{
    WeaponType type;
    int damage;
    util::Milliseconds attack_cooldown;
    WeaponKnockback knockback;
    int length;
    int offset;
    float invulnerability_window;

    int arc; // degrees
    int arc_speed; // degrees per second
    util::Seconds animation_time;

    int projectiles_per_attack;
    int delay_per_projectile;
    int projectile_spread;
    int projectile_speed;
};

Weapon GetWeapon(WeaponType type);

struct Projectile
{
    uint16_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool hostile;
    uint16_t owner;
    int damage;
    WeaponKnockback knockback;
    float invulnerability_window;
};

enum class ItemType : uint8_t
{
    None, Medpack
};

struct PlayerDefinition
{
public:
    static PlayerDefinition Get();

    int radius;
    int speed;

private:
    PlayerDefinition();
};

enum class Orientation
{
    North, South, East, West
};

class ConvoyDefinition
{
public:
    ConvoyDefinition();
    ConvoyDefinition(Orientation orientation);

    sf::FloatRect GetBounds();
    sf::FloatRect GetInteriorBounds();
    std::vector<sf::FloatRect> GetCollisions();

    sf::Vector2f Position;
    int Width;
    int Height;

    std::vector<sf::FloatRect> collisions;

private:
    sf::Vector2f origin;
    sf::FloatRect interior;

    void load(Orientation orientation);
};

struct MenuEventLink
{
    bool finish;
    uint16_t value;
    float weight;
};

struct MenuEventOption
{
    uint16_t parent;
    std::string text;
    std::vector<MenuEventLink> links;
};

struct MenuEventPage
{
    uint16_t page_index;
    std::string prompt;
    std::vector<MenuEventOption> options;
};

struct MenuEvent
{
    uint16_t event_id;
    std::string name;
    uint16_t current_page;
    std::vector<MenuEventPage> pages;
};

enum class ObstacleType
{
    SmallRock, LargeRock
};

struct Obstacle
{
    ObstacleType type;
    sf::FloatRect bounds;
};

struct Npc
{
    std::string name;
    std::string sprite_file;
    std::vector<std::string> dialog;
    sf::Vector2f position;
};

struct SpawnDetails
{
    EntityType type;
    int min;
    int max;
    std::vector<int> zone_scaling;
};

struct EnemyPack
{
    sf::Vector2f position;
    std::vector<SpawnDetails> spawns;
};

enum class RegionType : uint8_t
{
    StartingTown, Town, Leyline, Neutral, Secret, MenuEvent
};

struct RegionDefinition
{
    std::string name;
    uint16_t id;
    bool leyline;
    std::string background_file;
    sf::FloatRect bounds;
    ConvoyDefinition convoy;
    std::vector<Obstacle> obstacles;
    std::vector<Npc> npcs;
    std::vector<MenuEvent> events;
    std::vector<EnemyPack> enemy_packs;
};

struct Zone
{
    struct RegionNode
    {
        uint16_t id;
        RegionType type;
        sf::Vector2f coordinates;
    };

    struct Link
    {
        uint16_t start;
        uint16_t finish;
        double distance;
    };

    static constexpr int ZONE_WIDTH = 1800;
    static constexpr int ZONE_HEIGHT = 1000;

    std::vector<RegionNode> regions;
    std::vector<Link> links;
};

RegionDefinition GetRegionDefinition(RegionType region);
MenuEvent GetNextMenuEvent();
MenuEvent GetMenuEventById(uint16_t id);

} // namespace definitions
