/**************************************************************************************************
 *  File:       spritesheet.cpp
 *  Class:      Spritesheet
 *
 *  Purpose:    Spritesheet is a wrapper for an sf::Sprite that holds config information that
 *              describes the coordinates on the sheet of each frame.
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "spritesheet.h"
#include "resources.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

using std::cout, std::cerr, std::endl;

namespace client {

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string filename) : Spritesheet(filename, false) { }

Spritesheet::Spritesheet(std::string filename, bool tiled)
{
    Frame frame = {sf::IntRect(0, 0, 0, 0), sf::Vector2f{0, 0}};
    Animation animation{AnimationIdentifier{"Default", "None"}, 0, 0, 0, AnimationIdentifier{"Default", "None"}};

    animation_data.frames.push_back(frame);
    animation_data.animations[animation.identifier.group][animation.identifier.name] = animation;

    LoadAnimationData(filename);
    SetTiling(tiled);
}

void Spritesheet::Update(sf::Time elapsed)
{
    animation_timer += elapsed.asSeconds();
    if (current_animation.speed != 0 && animation_timer >= 1 / current_animation.speed)
    {
        animation_timer -= 1 / current_animation.speed;
        if (current_frame == current_animation.end)
        {
            SetAnimation(current_animation.next);
        }
        else
        {
            setFrame(current_frame + 1);
        }
    }
}

void Spritesheet::Draw()
{
    if (is_visible)
    {
        if (casts_shadow)
        {
            resources::GetWindow().draw(shadow);
        }

        resources::GetWindow().draw(sprite);

        if (debug_animation_print)
        {
            resources::GetWindow().draw(animation_text);
        }
    }
}

void Spritesheet::LoadAnimationData(std::string filename)
{
    std::filesystem::path path("../data/sprites/" + filename);
    if (!std::filesystem::exists(path))
    {
        cerr << "Could not open animation file: " << path << endl;
        return;
    }

    try
    {
        std::ifstream file(path);
        nlohmann::json j;
        file >> j;

        animation_data.frames = std::vector<Frame>(j["frames"].size());
        loadTexture(j["filename"]);

        for (auto& object : j["frames"])
        {
            Frame frame;

            sf::IntRect rect;
            rect.left = object["location"][0];
            rect.top = object["location"][1];
            rect.width = object["size"][0];
            rect.height = object["size"][1];

            sf::Vector2f origin;
            origin.x = object["origin"][0];
            origin.y = object["origin"][1];

            frame.bounds = rect;
            frame.origin = origin;
            animation_data.frames[object["index"]] = frame;
        }

        for (auto& group_object : j["animation_groups"])
        {
            std::string group_name = group_object["name"];
            for (auto& animation_object : group_object["animations"])
            {
                Animation animation;
                animation.identifier.name = animation_object["name"];
                animation.identifier.group = group_name;
                animation.start = animation_object["start_frame"];
                animation.end = animation_object["end_frame"];
                animation.speed = animation_object["animation_speed"];
                animation.next.name = animation_object["next_animation"]["animation"];
                animation.next.group = animation_object["next_animation"]["group"];

                animation_data.animations[animation.identifier.group][animation.identifier.name] = animation;
            }
        }

        animation_text.setFont(*resources::FontManager::GetFont("Vera"));
        animation_text.setCharacterSize(12);
        animation_text.setFillColor(sf::Color::White);
        animation_text.setOutlineColor(sf::Color::Black);
        animation_text.setOutlineThickness(1);

        Animation front = animation_data.animations.begin()->second.begin()->second;
        SetAnimation(front.identifier);
    }
    catch(const std::exception& e)
    {
        cerr << "Failed to parse animation data file: " << path << endl;
    }
}

void Spritesheet::SetShadow(bool shadows_on)
{
    casts_shadow = shadows_on;
    shadow_texture = resources::CreateShadow(animation_data.filepath, *texture);
    shadow.setTexture(*shadow_texture);
    shadow.setColor(sf::Color{0, 0, 0, 75});

    setFrame(current_frame);
}

sf::Sprite& Spritesheet::GetSprite()
{
    return sprite;
}

void Spritesheet::SetAnimation(AnimationIdentifier identifier)
{
    if (current_animation.identifier.group == identifier.group && current_animation.identifier.name == identifier.name)
    {
        return;
    }

    if (animation_data.animations.find(identifier.group) != animation_data.animations.end() &&
        animation_data.animations[identifier.group].find(identifier.name) != animation_data.animations[identifier.group].end())
    {
        current_animation = animation_data.animations[identifier.group][identifier.name];
        setFrame(current_animation.start);
        animation_timer = 0;

        animation_text.setString(current_animation.identifier.group + ", " + current_animation.identifier.name);
    }
    else
    {
        cerr << "Animation not found: " << current_animation.identifier.group << ", " << current_animation.identifier.name << endl;
    }
}

void Spritesheet::SetAnimation(std::string name, std::string group)
{
    SetAnimation(AnimationIdentifier{name, group});
}

void Spritesheet::SetAnimation(std::string name)
{
    SetAnimation(AnimationIdentifier{name, "None"});
}

void Spritesheet::SetPosition(float x, float y)
{
    sprite.setPosition(x, y);

    if (debug_animation_print)
    {
        animation_text.setPosition(sprite.getGlobalBounds().left, sprite.getGlobalBounds().top - animation_text.getGlobalBounds().height * 2);
    }

    if (casts_shadow)
    {
        shadow.setPosition(x + 4, y + 4);
    }
}

void Spritesheet::SetPosition(sf::Vector2f position)
{
    SetPosition(position.x, position.y);
}

void Spritesheet::SetDebugAnimationPrint(bool print)
{
    debug_animation_print = print;
}

void Spritesheet::CenterOrigin()
{
    if (animation_data.frames.empty())
    {
        sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
    }
}

void Spritesheet::SetTiling(bool tiled)
{
    if (texture != nullptr)
    {
        texture->setRepeated(tiled);

        if (tiled)
        {
            sprite.setTextureRect(sf::Rect{0, 0, 12000, 12000 });
        }
    }
}

void Spritesheet::SetVisible(bool visible)
{
    is_visible = visible;
}

bool Spritesheet::IsVisible()
{
    return is_visible;
}

bool Spritesheet::loadTexture(std::string filename)
{
    texture = resources::AllocTexture(filename);
    if (texture == nullptr)
    {
        return false;
    }

    //texture->setSmooth(true);

    sprite.setTexture(*texture);
    return true;
}

void Spritesheet::setFrame(unsigned frame)
{
    if (animation_data.frames.size() <= frame)
    {
        cerr << "Could not set frame: " << frame << endl;
        return;
    }

    current_frame = frame;
    sprite.setTextureRect(animation_data.frames[current_frame].bounds);
    sprite.setOrigin(animation_data.frames[current_frame].origin);

    if (casts_shadow)
    {
        shadow.setTextureRect(animation_data.frames[current_frame].bounds);
        shadow.setOrigin(animation_data.frames[current_frame].origin);
    }
}

} // client
