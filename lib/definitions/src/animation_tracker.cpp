/**************************************************************************************************
 *  File:       animation_tracker.cpp
 *  Class:      AnimationTracker
 *
 *  Purpose:    A server-side structure for tracking timings for animations
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "animation_tracker.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

using std::cout, std::cerr, std::endl;

namespace definitions {

AnimationTracker::AnimationTracker()
{
    Frame frame{};
    AnimationData animation{};
    animation.identifier = AnimationIdentifier{"Default", AnimationVariant::Default};
    animation.next = AnimationIdentifier{"Default", AnimationVariant::Default};

    spritesheet_data.frames.push_back(frame);
    spritesheet_data.animations[animation.identifier.name][animation.identifier.variant] = animation;
}

void AnimationTracker::Update(sf::Time elapsed)
{
    animation_timer += elapsed.asSeconds();
    if (current_animation.speed != 0 && animation_timer >= 1 / current_animation.speed)
    {
        animation_timer -= 1 / current_animation.speed;
        if (current_frame == current_animation.end_frame)
        {
            SetAnimation(current_animation.next);
        }
        else
        {
            setFrame(current_frame + 1);
        }
    }
}

std::string AnimationTracker::GetFilepath()
{
    return spritesheet_data.filepath;
}

Frame AnimationTracker::GetFrame()
{
    return spritesheet_data.frames[current_frame];
}

AnimationData AnimationTracker::GetAnimation()
{
    return current_animation;
}

void AnimationTracker::SetAnimation(AnimationIdentifier identifier)
{
    if (current_animation.identifier.name == identifier.name && current_animation.identifier.variant == identifier.variant)
    {
        if (current_frame != current_animation.end_frame)
        {
            return;
        }
    }

    if (spritesheet_data.animations.find(identifier.name) != spritesheet_data.animations.end() &&
        spritesheet_data.animations[identifier.name].find(identifier.variant) != spritesheet_data.animations[identifier.name].end())
    {
        current_animation = spritesheet_data.animations[identifier.name][identifier.variant];
        setFrame(current_animation.start_frame);
        animation_timer = 0;
    }
    else
    {
        cerr << "Animation not found: " << identifier.name << ", " << ToString(identifier.variant) << endl;
    }
}

void AnimationTracker::SetAnimation(AnimationName name, AnimationVariant variant)
{
    SetAnimation(AnimationIdentifier{name, variant});
}

void AnimationTracker::SetAnimation(AnimationName name)
{
    SetAnimation(AnimationIdentifier{name, AnimationVariant::Default});
}

void AnimationTracker::setFrame(unsigned frame)
{
    if (spritesheet_data.frames.size() <= frame)
    {
        cerr << "Could not set frame: " << frame << endl;
        return;
    }

    current_frame = frame;
}

AnimationTracker AnimationTracker::ConstructAnimationTracker(EntityType entity_type)
{
    switch (entity_type)
    {
        case EntityType::Player:
        {
            return ConstructAnimationTracker("../data/sprites/entities/player.json");
        }
        break;
        case EntityType::SmallDemon:
        {
            return ConstructAnimationTracker("../data/sprites/entities/small_demon.json");
        }
        break;
        case EntityType::Bat:
        {
            return ConstructAnimationTracker("../data/sprites/entities/bat.json");
        }
        break;
    }

    return AnimationTracker();
}

AnimationTracker AnimationTracker::ConstructAnimationTracker(std::string filepath)
{
    static std::map<std::string, AnimationTracker> animation_data_cache;

    if (animation_data_cache.find(filepath) != animation_data_cache.end())
    {
        return animation_data_cache[filepath];
    }

    AnimationTracker tracker;

    try
    {
        std::ifstream file(filepath);
        nlohmann::json j;
        file >> j;

        tracker.spritesheet_data.frames = std::vector<Frame>(j["frames"].size());
        tracker.spritesheet_data.filepath = j["filename"];

        for (auto& object : j["frames"])
        {
            Frame frame;

            sf::FloatRect bounds;
            bounds.left = object["location"][0];
            bounds.top = object["location"][1];
            bounds.width = object["size"][0];
            bounds.height = object["size"][1];

            sf::Vector2f origin;
            origin.x = object["origin"][0];
            origin.y = object["origin"][1];

            frame.draw_bounds = bounds;
            frame.origin = origin;

            if (object.find("attack_hitboxes") != object.end())
            {
                for (auto& j_hitbox : object["attack_hitboxes"])
                {
                    sf::FloatRect hitbox;
                    hitbox.left = j_hitbox["x"];
                    hitbox.top = j_hitbox["y"];
                    hitbox.width = j_hitbox["width"];
                    hitbox.height = j_hitbox["height"];

                    frame.attack_hitboxes.push_back(hitbox);
                }
            }

            frame.index = object["index"];
            tracker.spritesheet_data.frames[frame.index] = frame;
        }

        for (auto& j_animation : j["animations"])
        {
            std::string name = j_animation["name"];
            for (auto& j_variant : j_animation["variants"])
            {
                AnimationData animation;
                animation.identifier.name = name;
                animation.identifier.variant = ToVariant(j_variant["name"]);
                animation.start_frame = j_variant["start_frame"];
                animation.end_frame = j_variant["end_frame"];
                animation.speed = j_variant["animation_speed"];
                animation.next.name = j_variant["next_animation"]["animation"];
                animation.next.variant = ToVariant(j_variant["next_animation"]["variant"]);

                // Use a tiered approach, checking highest-level tiers first
                animation.collision_dimensions = sf::Vector2f{0, 0};
                if (j.find("collision_dimensions") != j.end())
                {
                    animation.collision_dimensions.x = j["collision_dimensions"]["x"];
                    animation.collision_dimensions.y = j["collision_dimensions"]["y"];
                }
                else if (j_animation.find("collision_dimensions") != j_animation.end())
                {
                    animation.collision_dimensions.x = j_animation["collision_dimensions"]["x"];
                    animation.collision_dimensions.y = j_animation["collision_dimensions"]["y"];
                }
                else if (j_variant.find("collision_dimensions") != j_variant.end())
                {
                    animation.collision_dimensions.x = j_variant["collision_dimensions"]["x"];
                    animation.collision_dimensions.y = j_variant["collision_dimensions"]["y"];
                }

                tracker.spritesheet_data.animations[animation.identifier.name][animation.identifier.variant] = animation;
            }
        }

        if (tracker.spritesheet_data.animations.empty())
        {
            throw std::runtime_error("Spritesheet has no animations: " + filepath + "\n");
        }

        animation_data_cache["filepath"] = tracker;
    }
    catch(const std::exception& e)
    {
        cerr << "AnimationTracker failed to parse file: " << filepath << ", " << e.what() << "\n";
    }

    return tracker;
}

} // server
