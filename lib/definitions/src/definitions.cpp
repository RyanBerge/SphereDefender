/**************************************************************************************************
 *  File:       region_definitions.cpp
 *
 *  Purpose:    Information about regions needed by both the client and the server
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "definitions.h"
#include "game_math.h"
#include "debug_overrides.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>

using std::cout, std::cerr, std::endl;

namespace definitions {
namespace {

class RegionInitializer
{
public:
    RegionInitializer()
    {
        LoadRegionData();
    }

    void LoadRegionData()
    {
        std::filesystem::path path("../data/definitions/regions");
        if (!std::filesystem::exists(path))
        {
            cerr << "Error loading regions: could not open directorty: " << path << endl;
            return;
        }

        for (const auto& region_file : std::filesystem::directory_iterator(path))
        {
            if (!region_file.is_regular_file())
            {
                continue;
            }

            try
            {
                std::ifstream file(region_file.path());
                nlohmann::json json;
                file >> json;

                RegionDefinition region;
                region.name = json["name"];
                region.background_file = "backgrounds/" + static_cast<std::string>(json["background"]) + ".json";

                region.bounds.left = json["bounds"]["x"];
                region.bounds.top = json["bounds"]["y"];
                region.bounds.width = json["bounds"]["width"];
                region.bounds.height = json["bounds"]["height"];

                if (json["convoy"]["orientation"] == "north")
                {
                    region.convoy = ConvoyDefinition(definitions::Orientation::North);
                }
                else if (json["convoy"]["orientation"] == "south")
                {
                    region.convoy = ConvoyDefinition(definitions::Orientation::South);
                }
                else if (json["convoy"]["orientation"] == "east")
                {
                    region.convoy = ConvoyDefinition(definitions::Orientation::East);
                }
                else if (json["convoy"]["orientation"] == "west")
                {
                    region.convoy = ConvoyDefinition(definitions::Orientation::West);
                }

                region.convoy.Position = sf::Vector2f(json["convoy"]["position"]["x"], json["convoy"]["position"]["y"]);
                if (json.find("wave_spawn_zone") != json.end())
                {
                    sf::FloatRect spawn_zone;
                    spawn_zone.left = json["wave_spawn_zone"]["x"];
                    spawn_zone.top = json["wave_spawn_zone"]["y"];
                    spawn_zone.width = json["wave_spawn_zone"]["width"];
                    spawn_zone.height = json["wave_spawn_zone"]["height"];

                    region.spawn_zone = spawn_zone;
                }

                for (auto& j_obstacle : json["obstacles"])
                {
                    ObstacleType type;
                    if (j_obstacle["type"] == "large_rock")
                    {
                        type = ObstacleType::LargeRock;
                    }

                    sf::FloatRect bounds{j_obstacle["bounds"]["x"], j_obstacle["bounds"]["y"], j_obstacle["bounds"]["width"], j_obstacle["bounds"]["height"]};
                    region.obstacles.push_back(Obstacle{type, bounds});
                }

                for (auto& j_npc : json["npcs"])
                {
                    Npc npc;
                    npc.name = j_npc["name"];
                    npc.sprite_file = "entities/" + static_cast<std::string>(j_npc["sprite"]) + ".json";
                    for (auto& j_dialog : j_npc["dialog"])
                    {
                        npc.dialog.push_back(j_dialog);
                    }
                    npc.position = sf::Vector2f{j_npc["position"]["x"], j_npc["position"]["y"]};

                    region.npcs.push_back(npc);
                }

                for (auto& j_enemy_type : json["enemies"])
                {
                    for (auto& j_pack : j_enemy_type["packs"])
                    {
                        PackIdentifier id;
                        id.name = j_pack["type"];
                        id.difficulty = 0; // TODO: figure out how to deal with increasing difficulty of regions...

                        EnemyPack pack = GetEnemyPackById(id);
                        pack.position = sf::Vector2f{j_pack["position"]["x"], j_pack["position"]["y"]};

                        region.enemy_packs.push_back(pack);
                    }
                }

                std::string type = json["type"];
                if (type == "starting_town")
                {
                    region.leyline = false;
                    Regions[RegionType::StartingTown][json["id"]] = region;
                }
                else if (type == "town")
                {
                    region.leyline = false;
                    Regions[RegionType::Town][json["id"]] = region;
                }
                else if (type == "leyline")
                {
                    region.leyline = true;
                    Regions[RegionType::Leyline][json["id"]] = region;
                }
                else if (type == "event")
                {
                    region.leyline = false;
                    Regions[RegionType::MenuEvent][json["id"]] = region;
                }
                else if (type == "neutral")
                {
                    region.leyline = false;
                    Regions[RegionType::Neutral][json["id"]] = region;
                }
                else if (type == "secret")
                {
                    region.leyline = false;
                    Regions[RegionType::Secret][json["id"]] = region;
                }
            }
            catch (const std::exception& e)
            {
                cerr << "Region failed to parse file: " << region_file.path() << endl;
            }
        }
    }

    std::map<RegionType, std::map<uint16_t, RegionDefinition>> Regions;
};

class EntityDefinitionManager
{
public:
    EntityDefinitionManager()
    {
        std::filesystem::path path("../data/definitions/entities");
        if (!std::filesystem::exists(path))
        {
            cerr << "Error loading entities: could not open directorty: " << path << endl;
            return;
        }

        for (const auto& entity_file : std::filesystem::directory_iterator(path))
        {
            if (!entity_file.is_regular_file())
            {
                continue;
            }

            try
            {
                std::ifstream file(entity_file.path());
                nlohmann::json json;
                file >> json;

                EntityDefinition entity;
                entity.behaviors = { { Behavior::Wandering, false }, { Behavior::Feeding, false },
                                     { Behavior::Hunting, false }, { Behavior::Stalking, false }, { Behavior::Dead, false } };

                entity.actions = { { Action::Knockback, false }, { Action::Sniffing, false }, { Action::Stunned, false } };

                entity.attacks = { { Action::Tackling, std::nullopt }, { Action::Leaping, std::nullopt }};

                entity.base_health = json["base_health"];
                entity.base_movement_speed = json["movement_speed"];
                entity.animation_definition_file = entity_file.path().filename().string();
                entity.walking_speed = json["walking_speed"];
                entity.feeding_range = json["feeding_range"];
                entity.siphon_rate = json["siphon_rate"];
                entity.steering_force = json["steering_force"];
                entity.repulsion_force = json["repulsion_force"];
                entity.repulsion_radius = json["repulsion_radius"];
                entity.acceleration = json["acceleration"];
                entity.deceleration = json["deceleration"];
                entity.wander_rest_time_min = json["wander_behavior"]["rest_min"];
                entity.wander_rest_time_max = json["wander_behavior"]["rest_max"];
                entity.swarming_rest_time_min = json["swarming_behavior"]["rest_min"];
                entity.swarming_rest_time_max = json["swarming_behavior"]["rest_max"];
                if (json.find("hopping_distance") != json.end())
                {
                    entity.hopping_distance = json["hopping_distance"];
                }
                if (json.find("hopping_cooldown") != json.end())
                {
                    entity.hopping_cooldown = json["hopping_cooldown"];
                }
                entity.aggro_range = json["aggro_range"];
                entity.combat_range = json["combat_range"];
                entity.close_quarters_range = json["close_quarters_range"];
                entity.leash_range = json["leash_range"];
                entity.base_aggression = json["base_aggression"];

                for (auto& behavior : json["behaviors"])
                {
                    if (behavior == "wandering")
                    {
                        entity.behaviors[Behavior::Wandering] = true;
                    }
                    else if (behavior == "feeding")
                    {
                        entity.behaviors[Behavior::Feeding] = true;
                    }
                    else if (behavior == "hunting")
                    {
                        entity.behaviors[Behavior::Hunting] = true;
                    }
                    else if (behavior == "stalking")
                    {
                        entity.behaviors[Behavior::Stalking] = true;
                    }
                    else if (behavior == "flocking")
                    {
                        entity.behaviors[Behavior::Flocking] = true;
                    }
                    else if (behavior == "swarming")
                    {
                        entity.behaviors[Behavior::Swarming] = true;
                    }
                    else if (behavior == "dead")
                    {
                        entity.behaviors[Behavior::Dead] = true;
                    }
                    else
                    {
                        cerr << "Unsupported behavior " << behavior << " listed in entity file: " << entity_file.path() << "\n";
                    }
                }

                for (auto& j_attack : json["attacks"])
                {
                    AttackDefinition attack_definition;
                    attack_definition.damage = j_attack["damage"];
                    attack_definition.range = j_attack["range"];
                    attack_definition.minimum_range = 0;
                    if (j_attack.find("minimum_range") != j_attack.end())
                    {
                        attack_definition.minimum_range = j_attack["minimum_range"];
                    }
                    attack_definition.cooldown = j_attack["cooldown"];
                    attack_definition.cooldown_timer = 0;
                    if (j_attack.find("duration") != j_attack.end())
                    {
                        attack_definition.duration = j_attack["duration"];
                    }
                    attack_definition.knockback_distance = j_attack["knockback_distance"];
                    if (j_attack.find("travel_distance") != j_attack.end())
                    {
                        attack_definition.travel_distance = j_attack["travel_distance"];
                    }

                    std::string name = j_attack["name"];
                    if (name == "tackle")
                    {
                        entity.attacks[Action::Tackling] = attack_definition;
                    }
                    else if (name == "leap")
                    {
                        entity.attacks[Action::Leaping] = attack_definition;
                    }
                    else if (name == "tail swipe")
                    {
                        entity.attacks[Action::TailSwipe] = attack_definition;
                    }
                    else
                    {
                        cerr << "Unsupported attack " << name << " listed in entity file: " << entity_file.path() << "\n";
                    }
                }

                for (auto& action : json["actions"])
                {
                    if (action == "knockback")
                    {
                        entity.actions[Action::Knockback] = true;
                    }
                    else if (action == "sniffing")
                    {
                        entity.actions[Action::Sniffing] = true;
                    }
                    else if (action == "stunned")
                    {
                        entity.actions[Action::Stunned] = true;
                    }
                    else
                    {
                        cerr << "Unsupported action " << action << " listed in entity file: " << entity_file.path() << "\n";
                    }
                }

                std::string type = entity_file.path().filename().string().substr(0, entity_file.path().filename().string().find_first_of('.'));
                if (type == "bat")
                {
                    definition_map[EntityType::Bat] = entity;
                }
                else if (type == "small_demon")
                {
                    definition_map[EntityType::SmallDemon] = entity;
                }
            }
            catch (const std::exception& e)
            {
                cerr << "Failed to parse entity file: " << entity_file.path() << endl;
            }
        }
    }

    std::map<EntityType, EntityDefinition> definition_map;
};

class PackDatabase
{
public:
    PackDatabase()
    {
        std::filesystem::path path("../data/definitions/enemy_pack.json");
        if (!std::filesystem::exists(path))
        {
            cerr << "File not found: " << path << "\n";
            return;
        }

        try
        {
            std::ifstream file(path);
            nlohmann::json json;
            file >> json;

            for (auto& j_pack : json["packs"])
            {
                EnemyPack pack;

                for (auto& j_enemy : j_pack["enemies"])
                {
                    SpawnDetails spawn_details;

                    spawn_details.min = j_enemy["min"];
                    spawn_details.max = j_enemy["max"];

                    std::string enemy_type = j_enemy["type"];
                    if (enemy_type == "small_demon")
                    {
                        spawn_details.type = EntityType::SmallDemon;
                    }
                    else if (enemy_type == "bat")
                    {
                        spawn_details.type = EntityType::Bat;
                    }
                    else
                    {
                        cerr << "Enemy type not supported: " << enemy_type << endl;
                    }

                    pack.spawns.push_back(spawn_details);
                }

                PackIdentifier id;
                id.difficulty = j_pack["difficulty"];
                id.name = j_pack["name"];

                spawn_map[id] = pack;
                spawns[id.difficulty].push_back(pack);
            }
        }
        catch (const std::exception& e)
        {
            cerr << "Failed to parse spawns file: " << path << endl;
        }
    }

    EnemyPack GetPackByDifficulty(PackDifficulty difficulty)
    {
        if (spawns[difficulty].size() == 0)
        {
            throw std::runtime_error("There are no spawns of the desired difficulty.");
        }

        std::shuffle(spawns[difficulty].begin(), spawns[difficulty].end(), util::RandomGenerator);
        return spawns[difficulty][0];
    }

    EnemyPack GetPackByName(PackIdentifier id)
    {
        if (spawn_map.find(id) == spawn_map.end())
        {
            throw (std::runtime_error("A spawn with this id does not exist."));
        }

        return spawn_map[id];
    }

private:
    std::map<PackIdentifier, EnemyPack> spawn_map;
    std::array<std::vector<EnemyPack>, 10> spawns;
};

PackDatabase& GetPackDatabase()
{
    static PackDatabase database;
    return database;
}

} // Anonymous namespace

RegionDefinition GetRegionDefinition(RegionType region)
{
    static RegionInitializer initializer;
    assert(initializer.Regions.find(region) != initializer.Regions.end());
    assert(initializer.Regions[region].find(0) != initializer.Regions[region].end());
    return initializer.Regions[region][0];
}

EnemyPack GetEnemyPackByDifficulty(PackDifficulty difficulty)
{
    return GetPackDatabase().GetPackByDifficulty(difficulty);
}

EnemyPack GetEnemyPackById(PackIdentifier id)
{
    return GetPackDatabase().GetPackByName(id);
}

EntityDefinition GetEntityDefinition(EntityType type)
{
    static EntityDefinitionManager manager;

    return manager.definition_map[type];
}

AnimationVariant ToVariant(std::string variant)
{
    static const std::map<std::string, AnimationVariant> variant_map = {
        { "Default", AnimationVariant::Default },
        { "North", AnimationVariant::North },
        { "South", AnimationVariant::South },
        { "East", AnimationVariant::East },
        { "West", AnimationVariant::West },
        { "Northeast", AnimationVariant::Northeast },
        { "Northwest", AnimationVariant::Northwest },
        { "Southeast", AnimationVariant::Southeast },
        { "Southwest", AnimationVariant::Southwest }
    };

    if (variant_map.find(variant) != variant_map.end())
    {
        return variant_map.at(variant);
    }
    else
    {
        std::cerr << "No animation variant with name: " << variant << "\n";
        return AnimationVariant::Default;
    }
}

std::string ToString(AnimationVariant variant)
{
    switch (variant)
    {
        case AnimationVariant::Default:
        {
            return "Default";
        }
        break;
        case AnimationVariant::North:
        {
            return "North";
        }
        break;
        case AnimationVariant::South:
        {
            return "South";
        }
        break;
        case AnimationVariant::East:
        {
            return "East";
        }
        break;
        case AnimationVariant::West:
        {
            return "West";
        }
        break;
        case AnimationVariant::Northeast:
        {
            return "Northeast";
        }
        break;
        case AnimationVariant::Northwest:
        {
            return "Northwest";
        }
        break;
        case AnimationVariant::Southeast:
        {
            return "Southeast";
        }
        break;
        case AnimationVariant::Southwest:
        {
            return "Southwest";
        }
        break;
    }

    return "None";
}

AnimationVariant GetAnimationVariant(util::Direction direction)
{
    switch (direction)
    {
        case util::Direction::East:
        {
            return AnimationVariant::East;
        }
        break;
        case util::Direction::Southeast:
        {
            return AnimationVariant::Southeast;
        }
        break;
        case util::Direction::South:
        {
            return AnimationVariant::South;
        }
        break;
        case util::Direction::Southwest:
        {
            return AnimationVariant::Southwest;
        }
        break;
        case util::Direction::West:
        {
            return AnimationVariant::West;
        }
        break;
        case util::Direction::Northwest:
        {
            return AnimationVariant::Northwest;
        }
        break;
        case util::Direction::North:
        {
            return AnimationVariant::North;
        }
        break;
        case util::Direction::Northeast:
        {
            return AnimationVariant::Northeast;
        }
        break;
        default:
        {
            return AnimationVariant::Default;
        }
    }
}

Weapon GetWeapon(WeaponType type)
{
    Weapon weapon;
    weapon.type = type;

    switch (type)
    {
        case WeaponType::Sword:
        {
            weapon.offset = 12;
            weapon.length = 43;
            weapon.damage = 50;
            weapon.knockback.distance = 35;
            weapon.knockback.duration = 0.1; // seconds
            weapon.knockback.stun_duration = 0.5; // seconds
            weapon.attack_cooldown = 250; // milliseconds between attacks
            weapon.arc = 146; // degrees;
            weapon.arc_speed = (18.0f / 2.0f) * weapon.arc; // degrees per second
            weapon.invulnerability_window = 0.3; // seconds
            weapon.animation_time = (7.0f / 18.0f);
        }
        break;
        case WeaponType::HitscanGun:
        {
            weapon.offset = -18;
            weapon.length = 7;
            weapon.damage = 50;
            weapon.attack_cooldown = 500; // milliseconds between attacks
            weapon.knockback.distance = 0;
            weapon.knockback.duration = 0;
            weapon.knockback.stun_duration = 0;
            weapon.projectiles_per_attack = 1;
            weapon.projectile_spread = 0; // degrees
            weapon.invulnerability_window = 0;
        }
        break;
        case WeaponType::BurstGun:
        {
            weapon.offset = -18;
            weapon.length = 7;
            weapon.damage = 25;
            weapon.attack_cooldown = 900; // milliseconds between attacks
            weapon.knockback.distance = 0;
            weapon.knockback.duration = 0;
            weapon.knockback.stun_duration = 0;
            weapon.projectiles_per_attack = 4;
            weapon.delay_per_projectile = 75; // milliseconds
            weapon.projectile_spread = 8; // degrees
            weapon.projectile_speed = 800; // pixels per second
            weapon.invulnerability_window = 0;
        }
        break;
    }

    return weapon;
}

PlayerDefinition::PlayerDefinition()
{
    radius = 18;
    speed = 100;
#ifndef NDEBUG
    if (debug::PlayerMovementSpeed.override)
    {
        speed = debug::PlayerMovementSpeed.value;
    }
#endif
}

PlayerDefinition PlayerDefinition::Get()
{
    static PlayerDefinition definition;
    return definition;
}

ConvoyDefinition::ConvoyDefinition() { }

ConvoyDefinition::ConvoyDefinition(Orientation orientation)
{
    load(orientation);
}

sf::FloatRect ConvoyDefinition::GetBounds()
{
    return sf::FloatRect(Position.x - origin.x, Position.y - origin.y, Width, Height);
}

sf::FloatRect ConvoyDefinition::GetInteriorBounds()
{
    sf::Vector2f relative_position{Position.x - origin.x, Position.y - origin.y};
    sf::FloatRect adjusted_interior(interior.left + relative_position.x, interior.top + relative_position.y, interior.width, interior.height);

    return adjusted_interior;
}

std::vector<sf::FloatRect> ConvoyDefinition::GetCollisions()
{
    sf::Vector2f relative_position{Position.x - origin.x, Position.y - origin.y};

    std::vector<sf::FloatRect> adjusted_collisions;

    for (auto& collision : collisions)
    {
        sf::FloatRect adjusted(collision.left + relative_position.x, collision.top + relative_position.y, collision.width, collision.height);
        adjusted_collisions.push_back(adjusted);
    }

    return adjusted_collisions;
}

void ConvoyDefinition::load(Orientation orientation)
{
    std::filesystem::path path;
    if (orientation == Orientation::North || orientation == Orientation::South)
    {
        path = std::filesystem::path("../data/sprites/entities/convoy_vertical.json");
    }
    else
    {
        path = std::filesystem::path("../data/sprites/entities/convoy_horizontal.json");
    }

    if (!std::filesystem::exists(path))
    {
        cerr << "Could not open animation file: " << path << endl;
        return;
    }

    try
    {
        std::ifstream file(path);
        nlohmann::json json;
        file >> json;

        Width = 0;
        Height = 0;

        for (auto& object : json["collisions"])
        {
            sf::FloatRect rect;

            rect.left = object["location"][0];
            rect.top = object["location"][1];
            rect.width = object["size"][0];
            rect.height = object["size"][1];

            if (rect.left + rect.width > Width)
            {
                Width = rect.left + rect.width;
            }

            if (rect.top + rect.height > Height)
            {
                Height = rect.top + rect.height;
            }

            collisions.push_back(rect);
        }

        interior.left = json["interior"]["location"][0];
        interior.top = json["interior"]["location"][1];
        interior.width = json["interior"]["size"][0];
        interior.height = json["interior"]["size"][1];

        origin.x = json["frames"][0]["origin"][0];
        origin.y = json["frames"][0]["origin"][1];
    }
    catch(const std::exception& e)
    {
        cerr << "Failed to parse convoy definition file: " << path << endl;
    }
}

class MenuEventInitializer
{
public:
    MenuEventInitializer()
    {
        std::filesystem::path path = std::filesystem::path("../data/definitions/menu_events.json");
        if (!std::filesystem::exists(path))
        {
            cerr << "Could not open definition file: " << path << endl;
            return;
        }

        try
        {
            std::ifstream file(path);
            nlohmann::json json;
            file >> json;

            for (auto& j_event : json["events"])
            {
                MenuEvent event;
                event.event_id = j_event["id"];
                event.current_page = 0;
                event.name = j_event["name"];

                for (auto& j_page : j_event["pages"])
                {
                    MenuEventPage page;
                    page.page_index = j_page["index"];
                    page.prompt = j_page["prompt"];

                    for (auto& j_option : j_page["options"])
                    {
                        MenuEventOption option;
                        option.parent = page.page_index;
                        option.text = j_option["text"];

                        for (auto& j_link : j_option["links"])
                        {
                            MenuEventLink link;
                            link.finish = j_link["finish"];
                            link.value = j_link["value"];
                            link.weight = j_link["weight"];

                            option.links.push_back(link);
                        }

                        page.options.push_back(option);
                    }

                    event.pages.push_back(page);
                }

                events.push_back(event);
            }
        }
        catch(const std::exception& e)
        {
            cerr << "Failed to parse menu event file: " << path << endl;
        }
    }

    MenuEvent Next()
    {
        static int next = 1;
        MenuEvent next_event = events[next];
        next = (next + 1) % events.size();
        return next_event;
    }

    MenuEvent GetEventById(uint16_t id)
    {
        for (auto& event : events)
        {
            if (event.event_id == id)
            {
                return event;
            }
        }

        cerr << "Could not find event with id: " << id << endl;
        return MenuEvent{};
    }

private:
    std::vector<MenuEvent> events;
};

MenuEventInitializer& getMenuEventInitializer()
{
    static MenuEventInitializer initializer;
    return initializer;
}

MenuEvent GetNextMenuEvent()
{
    return getMenuEventInitializer().Next();
}

MenuEvent GetMenuEventById(uint16_t id)
{
    return getMenuEventInitializer().GetEventById(id);
}

} // definitions
