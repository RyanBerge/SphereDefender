/**************************************************************************************************
 *  File:       pathfinding.h
 *
 *  Purpose:    Utilities for handling pathfinding
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <list>
#include "SFML/Graphics/Rect.hpp"

namespace util
{

struct PathingNode
{
    sf::Vector2f position;
    std::map<int, float> connections;

    float g_cost;
    float h_cost;
    float f_cost;

    int parent;
    bool visited;
    bool checked;
};

struct PathingGraph
{
    std::vector<PathingNode> nodes;
};

PathingGraph CreatePathingGraph(std::vector<sf::FloatRect> obstacles, sf::Vector2f entity_size);
PathingGraph AppendPathingGraph(sf::Vector2f start, sf::Vector2f finish, std::vector<sf::FloatRect> obstacles, sf::FloatRect entity_bounds, const PathingGraph& in_graph);
std::list<sf::Vector2f> GetPath(PathingGraph& graph);

struct DjikstraNode
{
    std::map<int, float> connections;
    bool visited = false;

    int parent;
    double cost;
};

std::vector<uint16_t> GetPath(std::vector<DjikstraNode> nodes, uint16_t start, uint16_t finish);

} // namespace util
