/**************************************************************************************************
 *  File:       spritesheet.h
 *  Class:      Spritesheet
 *
 *  Purpose:    Spritesheet is a wrapper for an sf::Sprite that holds config information that
 *              describes the coordinates on the sheet of each frame.
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "animation_tracker.h"
#include "definitions.h"
#include <memory>
#include <map>

namespace client {

class Spritesheet
{
public:
    Spritesheet();
    Spritesheet(std::string filename);
    Spritesheet(std::string filename, bool tiled);

    void Update(sf::Time elapsed);
    void Draw();

    sf::Sprite& GetSprite();
    void LoadAnimationData(std::string filename);
    void SetShadow(bool shadows_on);
    void SetAnimation(definitions::AnimationIdentifier identifier);
    void SetAnimation(definitions::AnimationName name, definitions::AnimationVariant variant);
    void SetAnimation(definitions::AnimationName name);
    definitions::AnimationIdentifier GetAnimation();
    sf::Vector2f GetCollisionDimensions();
    void SetPosition(float x, float y);
    void SetPosition(sf::Vector2f position);
    void SetDebugAnimationPrint(bool print);
    void SetTiling(bool tiled);
    void SetVisible(bool visible);
    bool IsVisible();

    static definitions::AnimationVariant GetAnimationVariant(util::Direction direction);

private:
    sf::Sprite sprite;
    std::shared_ptr<sf::Texture> texture;
    definitions::AnimationTracker animation_tracker;

    bool casts_shadow = false;
    sf::Sprite shadow;
    std::shared_ptr<sf::Texture> shadow_texture;

    unsigned current_frame_index = UINT_MAX;
    bool debug_animation_print = false;
    sf::Text animation_text;

    bool is_visible = true;

    bool loadTexture(std::string filename);
    void setFrame();
    void setFrame(bool initialize);
};

} // client
