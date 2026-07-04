/**************************************************************************************************
 *  File:       definitions.h
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <map>
#include <vector>
#include <list>
#include <string>
#include <optional>
#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "skill_definitions.h"

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
    util::DistanceUnits range;
    util::DistanceUnits minimum_range;
    float damage;
    util::Seconds cooldown;
    util::Seconds cooldown_timer;
    util::Seconds duration;
    util::DistanceUnits knockback_distance;
    util::DistanceUnits travel_distance;
};

struct AttackEvent
{
    uint16_t source_id;
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
    util::Seconds hopping_cooldown;
    int aggro_range;
    int combat_range;
    int close_quarters_range;
    int leash_range;
    float base_aggression;
};

EntityDefinition GetEntityDefinition(EntityType type);

using AnimationName = std::string;
using FramesPerSecond = float;

enum class AnimationVariant
{
    Default,
    North, South, East, West,
    Northeast, Northwest, Southeast, Southwest,
    Active
};

AnimationVariant ToVariant(std::string variant);
std::string ToString(AnimationVariant variant);
AnimationVariant GetAnimationVariant(util::Direction direction);

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
    std::vector<unsigned> hitbox_frames;
    sf::Vector2f collision_dimensions;
    AnimationIdentifier next;
};

struct Frame
{
    unsigned index;
    sf::Vector2f origin; // The position in the frame to be considered the origin point, relative to the draw bounds
    sf::FloatRect draw_bounds; // The bounds of the frame within the spritesheet
    std::vector<sf::FloatRect> attack_hitboxes; // The attack hitboxes, if any, for this frame, relative to the origin
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

enum class SkillType
{
    None,
    DodgeRoll, Lunge
};

struct Skill
{
    SkillType type;
    int row;
    std::string name;
    std::string description;
    float cooldown;
};

enum class PlayerClassType
{
    Melee, Ranged
};

struct PlayerClass
{
    PlayerClassType type;
    std::vector<std::vector<Skill>> skills;
};

PlayerClass GetPlayerClass(PlayerClassType type);

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
    bool shop;
    uint16_t shop_id;
};

enum class InventoryItemType : uint8_t
{
    None, Medpack
};

struct InventoryItem
{
    InventoryItemType type;
};

enum class ConsumableItemType
{
    Diary
};

enum class ShopItemType : uint8_t
{
    Medpack, Diary
};

struct ShopItem
{
    uint16_t id;
    ShopItemType type;
    uint16_t cost;
    bool stale;
    bool grants_inventory_item;
    InventoryItemType inventory_item_type;
    bool grants_consumable_item;
    ConsumableItemType consumable_type;
};

ShopItem CreateShopItem(std::string type_name);
ShopItemType GetShopItemType(std::string type_name);

struct Shop
{
    uint16_t id;
    std::list<ShopItem> stock;
};

enum class LootItemType : uint8_t
{
    Scrap
};

struct LootItem
{
    uint16_t id;
    LootItemType type;
    uint16_t value;
    sf::Vector2f position;
    bool spawned;
};

std::string ToString(InventoryItemType item_type);
std::string ToString(ShopItemType item_type);
std::string ToString(LootItemType item_type);

std::string GetAnimationFilename(ShopItemType type);

MenuEvent GetNextMenuEvent();
MenuEvent GetMenuEventById(uint16_t id);

using PackDifficulty = int;

struct PackIdentifier
{
    std::string name;
    PackDifficulty difficulty;

    bool operator==(const PackIdentifier& other) const
    {
        return (difficulty == other.difficulty) && (name == other.name);
    }

    bool operator<(const PackIdentifier& other) const
    {
        if (difficulty != other.difficulty)
        {
            return difficulty < other.difficulty;
        }

        return name < other.name;
    }
};

struct SpawnDetails
{
    EntityType type;
    int min;
    int max;
};

struct EnemyPack
{
    sf::Vector2f position;
    std::vector<SpawnDetails> spawns;
};

EnemyPack GetEnemyPackByDifficulty(PackDifficulty difficulty);
EnemyPack GetEnemyPackById(PackIdentifier id);

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
    sf::FloatRect spawn_zone;
    std::vector<Obstacle> obstacles;
    std::vector<Npc> npcs;
    std::vector<definitions::Shop> shops;
    std::vector<LootItem> loot_items;
    std::map<definitions::LootItemType, std::string> loot_item_spritesheets;
    std::vector<MenuEvent> events;
    std::vector<EnemyPack> enemy_packs;
};

struct Zone
{
    using Difficulty = float;
    struct RegionNode
    {
        uint16_t id;
        RegionType type;
        sf::Vector2f coordinates;
        Difficulty difficulty;
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

} // namespace definitions
