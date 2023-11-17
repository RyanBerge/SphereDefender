/**************************************************************************************************
 *  File:       animation_tracker.h
 *  Class:      AnimationTracker
 *
 *  Purpose:    A structure for tracking timings for animations
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/System/Time.hpp>
#include "definitions.h"

namespace definitions {

class AnimationTracker
{
public:
    static AnimationTracker ConstructAnimationTracker(definitions::EntityType entity_type);
    static AnimationTracker ConstructAnimationTracker(std::string filepath);

    AnimationTracker();

    void Update(sf::Time elapsed);

    std::string GetFilepath();
    definitions::Frame GetFrame();
    definitions::AnimationData GetAnimation();
    void SetAnimation(AnimationIdentifier identifier);
    void SetAnimation(AnimationName name, AnimationVariant variant);
    void SetAnimation(AnimationName name);

private:
    void setFrame(unsigned frame);

    definitions::SpritesheetData spritesheet_data;

    definitions::AnimationData current_animation{};
    float animation_timer = 0;
    unsigned current_frame = 0;

};

} // definitions
