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
#include "game_manager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

using std::cout, std::cerr, std::endl;

namespace client {

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string filename)
{
    LoadTexture(filename);
}

Spritesheet::Spritesheet(std::string filename, bool tiled)
{
    LoadTexture(filename);
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
            current_animation = animation_data.animations[current_animation.next];
            setFrame(0);
        }
        else
        {
            setFrame(current_frame + 1);
        }
    }
}

void Spritesheet::Draw()
{
    GameManager::GetInstance().Window.draw(sprite);
    if (texture->isRepeated())
    {
        GameManager::GetInstance().Window.draw(tiled_sprite);
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

    std::ifstream file(path);
    nlohmann::json j;
    file >> j;

    animation_data.frames = std::vector<sf::IntRect>(j["frames"].size());
    LoadTexture(j["filename"]);

    for (auto& object : j["frames"])
    {
        sf::IntRect rect;
        rect.left = object["location"][0];
        rect.top = object["location"][1];
        rect.width = object["size"][0];
        rect.height = object["size"][1];

        animation_data.frames[object["index"]] = rect;
    }

    for (auto& object : j["animations"])
    {
        Animation animation;
        animation.start = object["start_frame"];
        animation.end = object["end_frame"];
        animation.speed = object["animation_speed"];
        animation.next = object["next_animation"];

        animation_data.animations[object["name"]] = animation;
    }
}

bool Spritesheet::LoadTexture(std::string filename)
{
    texture = resources::AllocTexture(filename);
    if (texture == nullptr)
    {
        return false;
    }

    sprite.setTexture(*texture);
    return true;
}

sf::Sprite& Spritesheet::GetSprite()
{
    return sprite;
}

void Spritesheet::SetAnimation(std::string animation_name)
{
    if (animation_data.animations.find(animation_name) != animation_data.animations.end())
    {
        current_animation = animation_data.animations[animation_name];
        setFrame(current_animation.start);
    }
    else
    {
        cerr << "Animation not found: " << animation_name << endl;
    }
}

void Spritesheet::SetPosition(float x, float y)
{
    sprite.setPosition(x, y);
    if (texture != nullptr)
    {
        tiled_sprite.setPosition(x + texture->getSize().x / 2, y + texture->getSize().y / 2);
    }
}

void Spritesheet::SetPosition(sf::Vector2f position)
{
    SetPosition(position.x, position.y);
}

void Spritesheet::SetTiling(bool tiled)
{
    texture->setRepeated(tiled);
    tiled_sprite.setTexture(*texture);

    sprite.setTextureRect(sf::Rect{0, 0, 12000, 12000 });
    tiled_sprite.setTextureRect(sf::Rect{0, 0, 12000, 12000 });
}

void Spritesheet::setFrame(unsigned frame)
{
    current_frame = frame;
    sprite.setTextureRect(animation_data.frames[current_frame]);
    if (animation_data.frames.size() <= frame)
    {
        cerr << "Could not set frame: " << frame << endl;
    }
}

} // client
