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
    bool LoadTexture(std::string filename);
    void LoadAnimationData(std::string filename);

    void SetAnimation(std::string animation_name);
    void SetPosition(float x, float y);
    void SetPosition(sf::Vector2f position);
    void SetTiling(bool tiled);

private:
    struct Animation
    {
        int start;
        int end;
        float speed;
        std::string next;
    };

    struct AnimationData
    {
        std::string filepath;
        std::vector<sf::IntRect> frames;
        std::map<std::string, Animation> animations;
    };

    sf::Sprite sprite;
    std::shared_ptr<sf::Texture> texture;
    AnimationData animation_data;

    Animation current_animation{};
    int current_frame = 0;
    float animation_timer = 0;

    sf::Sprite tiled_sprite;

    void setFrame(int frame);
};

} // client
