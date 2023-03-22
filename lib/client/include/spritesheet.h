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
    void SetAnimation(std::string animation_name);
    void SetPosition(float x, float y);
    void SetPosition(sf::Vector2f position);
    void SetDebugAnimationPrint(bool print);
    void CenterOrigin();
    void SetTiling(bool tiled);
    void SetVisible(bool visible);
    bool IsVisible();

private:
    struct Frame
    {
        sf::IntRect bounds;
        sf::Vector2f origin;
    };

    struct Animation
    {
        std::string name;
        unsigned start;
        unsigned end;
        float speed;
        std::string next;
    };

    struct AnimationData
    {
        std::string filepath;
        std::vector<Frame> frames;
        std::map<std::string, Animation> animations;
    };

    sf::Sprite sprite;
    std::shared_ptr<sf::Texture> texture;
    AnimationData animation_data;

    bool casts_shadow = false;
    sf::Sprite shadow;
    std::shared_ptr<sf::Texture> shadow_texture;

    Animation current_animation{};
    unsigned current_frame = 0;
    float animation_timer = 0;
    bool debug_animation_print = false;
    sf::Text animation_text;

    bool is_visible = true;

    bool loadTexture(std::string filename);
    void setFrame(unsigned frame);
};

} // client
