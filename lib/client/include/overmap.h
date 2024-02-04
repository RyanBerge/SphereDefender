/**************************************************************************************************
 *  File:       overmap.h
 *  Class:      Overmap
 *
 *  Purpose:    The map that allows players to advance the convoy to different regions
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "cursor_button.h"
#include "toggle_button.h"
#include "definitions.h"
#include "pathfinding.h"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

namespace client
{

class Overmap
{
public:
    Overmap();

    void Load(definitions::Zone zone);
    void Update(sf::Time elapsed);
    void Draw();

    void SetActive(bool is_active, float battery_level = 0);
    bool IsActive();
    void SetRegion(uint16_t region_id);
    uint16_t GetRegion();
    sf::Vector2f GetBaseVoteIndicatorPosition(uint8_t vote);

    void OnMouseMove(sf::Event::MouseMoveEvent event);
    void OnMouseDown(sf::Event::MouseButtonEvent event);
    void OnMouseUp(sf::Event::MouseButtonEvent event);

private:
    struct Node
    {
        definitions::Zone::RegionNode definition;
        CursorButton button;
        bool visited;
    };

    struct Link
    {
        definitions::Zone::Link definition;
        sf::RectangleShape line;
        sf::RectangleShape highlight;
    };

    void castVote(bool toggled);
    Node getNodeById(uint16_t id);

    ToggleButton confirm_button;
    CursorButton cancel_button;

    bool active = false;
    float battery;
    uint16_t current_region;

    sf::RectangleShape frame;
    sf::RectangleShape map_area;
    sf::Text information;

    Spritesheet marker;
    std::vector<Node> region_nodes;
    std::vector<Link> links;
    std::vector<util::DjikstraNode> node_graph;

    uint16_t current_vote;

    void onClickNode(uint16_t region_id);
    void onHoverNodeEnter(uint16_t node_id);
    void onHoverNodeExit();
};

} // namespace client
