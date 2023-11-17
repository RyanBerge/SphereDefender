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

using definitions::AnimationName;
using definitions::AnimationVariant;
using definitions::AnimationIdentifier;
using definitions::Frame;

namespace client {

Spritesheet::Spritesheet()
{
}

Spritesheet::Spritesheet(std::string filename) : Spritesheet(filename, false) { }

Spritesheet::Spritesheet(std::string filename, bool tiled)
{
    LoadAnimationData(filename);
    SetTiling(tiled);
}

void Spritesheet::LoadAnimationData(std::string filename)
{
    animation_tracker = definitions::AnimationTracker::ConstructAnimationTracker("../data/sprites/" + filename);
    loadTexture(animation_tracker.GetFilepath());
    setFrame(true);

    animation_text.setFont(*resources::FontManager::GetFont("Vera"));
    animation_text.setCharacterSize(12);
    animation_text.setFillColor(sf::Color::White);
    animation_text.setOutlineColor(sf::Color::Black);
    animation_text.setOutlineThickness(1);
}

void Spritesheet::Update(sf::Time elapsed)
{
    animation_tracker.Update(elapsed);
    setFrame();
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

void Spritesheet::SetShadow(bool shadows_on)
{
    casts_shadow = shadows_on;
    shadow_texture = resources::CreateShadow(animation_tracker.GetFilepath(), *texture);
    shadow.setTexture(*shadow_texture);
    shadow.setColor(sf::Color{0, 0, 0, 75});

    setFrame(true);
}

sf::Sprite& Spritesheet::GetSprite()
{
    return sprite;
}

void Spritesheet::SetAnimation(AnimationIdentifier identifier)
{
    animation_tracker.SetAnimation(identifier);
    animation_text.setString(identifier.name);
    setFrame();
}

void Spritesheet::SetAnimation(AnimationName name, AnimationVariant variant)
{
    SetAnimation(AnimationIdentifier{name, variant});
}

void Spritesheet::SetAnimation(AnimationName name)
{
    SetAnimation(AnimationIdentifier{name, AnimationVariant::Default});
}

AnimationIdentifier Spritesheet::GetAnimation()
{
    return animation_tracker.GetAnimation().identifier;
}

sf::Vector2f Spritesheet::GetCollisionDimensions()
{
    return animation_tracker.GetAnimation().collision_dimensions;
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

    sprite.setTexture(*texture);
    return true;
}

void Spritesheet::setFrame(bool initialize)
{
    definitions::Frame new_frame = animation_tracker.GetFrame();

    if (initialize || new_frame.index != current_frame_index)
    {
        current_frame_index = new_frame.index;
        sprite.setTextureRect(static_cast<sf::IntRect>(new_frame.draw_bounds));
        sprite.setOrigin(new_frame.origin);

        if (casts_shadow)
        {
            shadow.setTextureRect(static_cast<sf::IntRect>(static_cast<sf::IntRect>(new_frame.draw_bounds)));
            shadow.setOrigin(new_frame.origin);
        }
    }
}

void Spritesheet::setFrame()
{
    setFrame(false);
}

AnimationVariant Spritesheet::GetAnimationVariant(util::Direction direction)
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

} // client
