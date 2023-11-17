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

enum class Behavior
{
    None,
    Wandering,
    Feeding,
    Hunting,
    Stalking,
    Flocking,
    Swarming,
    Dead
};

enum class Action
{
    None,
    Tackling,
    Knockback,
    Sniffing,
    Stunned,
    Leaping,
    Hopping,
    TailSwipe
};

struct AttackDefinition
{
    float range;
    float damage;
    util::Seconds cooldown;
    util::Seconds cooldown_timer;
    util::Seconds duration;
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
    std::string animation_definition_file;
    int base_health;
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
    util::Seconds wander_rest_time_min;
    util::Seconds wander_rest_time_max;
    util::Seconds swarming_rest_time_min;
    util::Seconds swarming_rest_time_max;
    int hopping_distance;
    int aggro_range;
    int combat_range;
    int close_quarters_range;
    int leash_range;
    float base_aggression;
    util::Seconds leap_windup_time;
    util::Seconds leap_time;
    util::Seconds leap_rest_time;
    util::Seconds hop_windup_time;
    util::Seconds hop_time;
    util::Seconds tail_swipe_time;
    sf::FloatRect attack_hitbox;
};

EntityDefinition GetEntityDefinition(EntityType type);

using AnimationName = std::string;
using FramesPerSecond = float;

enum class AnimationVariant
{
    Default,
    North, South, East, West,
    Northeast, Northwest, Southeast, Southwest
};

AnimationVariant ToVariant(std::string variant);
std::string ToString(AnimationVariant variant);

struct AnimationIdentifier
{
    AnimationName name;
    AnimationVariant variant;
};

struct AnimationData
{
    AnimationIdentifier identifier;
    unsigned start_frame;
    unsigned end_frame;
    FramesPerSecond speed;
    sf::Vector2f collision_dimensions;
    AnimationIdentifier next;
};

struct Frame
{
    unsigned index;
    sf::Vector2f origin; // The position in the frame to be considered the origin point, relative to the draw bounds
    sf::FloatRect draw_bounds; // The bounds of the frame within the spritesheet
    std::vector<sf::FloatRect> attack_hitboxes; // The attack hitboxes, if any, for this frame, relative origin
};

struct SpritesheetData
{
    std::string filepath;
    std::vector<Frame> frames;
    std::map<AnimationName, std::map<AnimationVariant, AnimationData>> animations;
};

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
