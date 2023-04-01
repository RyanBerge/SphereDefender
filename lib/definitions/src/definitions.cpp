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

using std::cout, std::cerr, std::endl;

namespace definitions {
namespace {

class RegionInitializer
{
public:
    RegionInitializer()
    {
        LoadEntityData();
        LoadRegionData();
    }

    void LoadEntityData()
    {
        std::filesystem::path path("../data/definitions/enemy_pack.json");
        if (!std::filesystem::exists(path))
        {
            cerr << "Error loading enemy packs: could not open file: " << path << endl;
            return;
        }

        try
        {
            std::ifstream file(path);
            nlohmann::json json;
            file >> json;

            for (auto& j_pack : json["packs"])
            {
                std::vector<SpawnDetails> pack;

                for (auto& j_enemy : j_pack["enemies"])
                {
                    SpawnDetails details;
                    details.min = j_enemy["min"];
                    details.max = j_enemy["max"];
                    for (int scaling : j_enemy["zone_scaling"])
                    {
                        details.zone_scaling.push_back(scaling);
                    }

                    std::string enemy_type = j_enemy["type"];
                    if (enemy_type == "small_demon")
                    {
                        details.type = EntityType::SmallDemon;
                    }
                    else if (enemy_type == "bat")
                    {
                        details.type = EntityType::Bat;
                    }
                    else
                    {
                        cerr << "Enemy type not supported: " << enemy_type << endl;
                    }

                    pack.push_back(details);
                }

                EnemyPacks[j_pack["type"]] = pack;
            }
        }
        catch(const std::exception& e)
        {
            cerr << "Initializer failed to parse file: " << path << endl;
        }
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
                        EnemyPack pack;
                        pack.position = sf::Vector2f{j_pack["position"]["x"], j_pack["position"]["y"]};

                        pack.spawns = EnemyPacks[j_pack["type"]];
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
    std::map<std::string, std::vector<SpawnDetails>> EnemyPacks;
};

class EntityDefinitionManager
{
public:
    EntityDefinitionManager()
    {
//        EntityDefinition small_demon;
//        small_demon.base_movement_speed = 105;
//        small_demon.size = sf::Vector2f{25, 25};
//        small_demon.attack_damage = 30;
//        small_demon.attack_range = 36;
//        small_demon.minimum_leap_range = 75;
//        small_demon.maximum_leap_range = 200;
//        small_demon.leaping_speed = 750;
//        small_demon.leaping_distance = 225;
//        small_demon.feeding_range = 18;
//        small_demon.attack_distance = 30;
//        small_demon.attack_duration = 0.25;
//        small_demon.attack_cooldown = 0.5;
//
//        definition_map[EntityType::SmallDemon] = small_demon;

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

                entity.actions = { { Action::Tackling, false }, { Action::Knockback, false },
                                   { Action::Sniffing, false }, { Action::Stunned, false }, { Action::Leaping, false } };

                entity.base_movement_speed = json["movement_speed"];
                entity.walking_speed = json["walking_speed"];
                entity.feeding_range = json["feeding_range"];
                entity.steering_force = json["steering_force"];
                entity.repulsion_force = json["repulsion_force"];
                entity.repulsion_radius = json["repulsion_radius"];
                entity.acceleration = json["acceleration"];
                entity.deceleration = json["deceleration"];
                entity.wander_rest_time_min = json["wander_behavior"]["rest_min"];
                entity.wander_rest_time_max = json["wander_behavior"]["rest_max"];
                entity.aggro_range = json["aggro_range"];
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
                    else if (behavior == "dead")
                    {
                        entity.behaviors[Behavior::Dead] = true;
                    }
                    else
                    {
                        cerr << "Unsupported behavior " << behavior << " listed in entity file: " << entity_file.path() << "\n";
                    }
                }

                for (auto& action : json["actions"])
                {
                    if (action == "tackling")
                    {
                        entity.actions[Action::Tackling] = true;
                    }
                    else if (action == "knockback")
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
                    else if (action == "leaping")
                    {
                        entity.actions[Action::Leaping] = true;
                    }
                    else
                    {
                        cerr << "Unsupported action " << action << " listed in entity file: " << entity_file.path() << "\n";
                    }
                }

                std::string type = entity_file.path().filename().string().substr(0, entity_file.path().filename().string().find_first_of('.'));

                std::filesystem::path sprite_path("../data/sprites/entities/" + type + ".json");
                if (!std::filesystem::exists(sprite_path))
                {
                    cerr << "Error loading entities: could not open file: " << sprite_path << endl;
                    return;
                }

                try
                {
                    std::ifstream sprite_file(sprite_path);
                    nlohmann::json sprite_json;
                    sprite_file >> sprite_json;

                    entity.hitbox.x = sprite_json["pathing_hitbox"]["x"];
                    entity.hitbox.y = sprite_json["pathing_hitbox"]["y"];
                }
                catch (const std::exception& e)
                {
                    cerr << "Failed to parse sprite file: " << sprite_path << endl;
                }

                if (type == "bat")
                {
                    definition_map[EntityType::Bat] = entity;
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

} // Anonymous namespace

EntityDefinition GetEntityDefinition(EntityType type)
{
    static EntityDefinitionManager manager;

    return manager.definition_map[type];
}

Weapon GetWeapon(WeaponType type)
{
    Weapon weapon;
    weapon.type = type;

    switch (type)
    {
        case WeaponType::Sword:
        {
            weapon.offset = -18;
            weapon.length = 20;
            weapon.damage = 50;
            weapon.knockback.distance = 35;
            weapon.knockback.duration = 0.1; // seconds
            weapon.attack_cooldown = 250; // milliseconds between attacks
            weapon.arc = 90; // degrees;
            weapon.arc_speed = 360; // degrees per second
            weapon.invulnerability_window = 0.3; // seconds
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

RegionDefinition GetRegionDefinition(RegionType region)
{
    static RegionInitializer initializer;
    assert(initializer.Regions.find(region) != initializer.Regions.end());
    assert(initializer.Regions[region].find(0) != initializer.Regions[region].end());
    return initializer.Regions[region][0];
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
