/**************************************************************************************************
 *  File:       pathfinding.h
 *
 *  Purpose:    Utilities for handling pathfinding
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/

#include "pathfinding.h"
#include "game_math.h"
#include <iostream>

using std::cout, std::endl;

namespace util {
namespace {

[[maybe_unused]] void printPathingGraph(PathingGraph graph)
{
    for (unsigned i = 0; i < graph.nodes.size(); ++i)
    {
        cout << "Node " << i << ": X=" << graph.nodes[i].position.x << ", Y=" << graph.nodes[i].position.y << "\n";
        for (auto& connection : graph.nodes[i].connections)
        {
            cout << "    Connection: " << connection.first << ", Cost: " << connection.second << "\n";
        }
        cout << endl;
    }
}

} // anonymous namespace

PathingGraph CreatePathingGraph(sf::Vector2f start, sf::Vector2f finish, std::vector<sf::FloatRect> obstacles, sf::Vector2f entity_size)
{
    PathingGraph graph;

    PathingNode start_node{};
    start_node.position = start;
    graph.nodes.push_back(start_node); // start: index == 0

    PathingNode finish_node{};
    finish_node.position = finish;
    graph.nodes.push_back(finish_node); // finish: index == 1

    for (auto& rect : obstacles)
    {
        PathingNode upper_left{};
        upper_left.position = sf::Vector2f{rect.left - (entity_size.x / 2 + 1), rect.top - (entity_size.y / 2 + 1)};
        graph.nodes.push_back(upper_left);

        PathingNode upper_right{};
        upper_right.position = sf::Vector2f{rect.left + rect.width + (entity_size.x / 2 + 1), rect.top - (entity_size.y / 2 + 1)};
        graph.nodes.push_back(upper_right);

        PathingNode lower_left{};
        lower_left.position = sf::Vector2f{rect.left - (entity_size.x / 2 + 1), rect.top + rect.height + (entity_size.y / 2 + 1)};
        graph.nodes.push_back(lower_left);

        PathingNode lower_right{};
        lower_right.position = sf::Vector2f{rect.left + rect.width + (entity_size.x / 2 + 1), rect.top + rect.height + (entity_size.y / 2 + 1)};
        graph.nodes.push_back(lower_right);
    }

    for (unsigned i = 0; i < graph.nodes.size() - 1; ++i)
    {
        PathingNode& current_node = graph.nodes[i];
        for (unsigned j = i + 1; j < graph.nodes.size(); ++j)
        {
            PathingNode& other_node = graph.nodes[j];

            sf::Vector2f path_vector = other_node.position - current_node.position;
            float length = std::hypot(path_vector.x, path_vector.y);
            sf::Vector2f left_orthogonal{-path_vector.y / length * entity_size.x / 2, path_vector.x / length * entity_size.y / 2};
            sf::Vector2f right_orthogonal{path_vector.y / length * entity_size.x / 2, -path_vector.x / length * entity_size.y / 2};

            LineSegment left_bound{current_node.position + left_orthogonal, other_node.position + left_orthogonal};
            LineSegment right_bound{current_node.position + right_orthogonal, other_node.position + right_orthogonal};

            bool line_of_sight = true;
            for (auto& rect : obstacles)
            {
                if (Intersects(rect, left_bound))
                {
                    line_of_sight = false;
                    break;
                }

                if (Intersects(rect, right_bound))
                {
                    line_of_sight = false;
                    break;
                }
            }

            if (line_of_sight)
            {
                float distance = Distance(current_node.position, other_node.position);
                current_node.connections[j] = distance;
                other_node.connections[i] = distance;
            }
        }

        current_node.h_cost = Distance(current_node.position, graph.nodes[1].position);
    }

    return graph;
}

std::list<sf::Vector2f> GetPath(PathingGraph& graph)
{
    std::list<sf::Vector2f> path;
    std::list<int> open_list;

    if (graph.nodes[0].connections.find(1) != graph.nodes[0].connections.end())
    {
        path.push_front(graph.nodes[1].position);
        return path;
    }

    int current_index = 0;
    PathingNode& start_node = graph.nodes[current_index];
    start_node.checked = true;

    open_list.push_back(current_index);

    bool path_found = false;
    while (true)
    {
        if (open_list.empty())
        {
            // No path available?
            break;
        }
        // Get node with lowest f-cost from the open list
        current_index = open_list.front();
        PathingNode& current_node = graph.nodes[current_index];
        for (auto index : open_list)
        {
            if (graph.nodes[index].f_cost < current_node.f_cost)
            {
                current_index = index;
                current_node = graph.nodes[current_index];
            }
        }

        if (current_index == 1)
        {
            path_found = true;
            break;
        }

        current_node.visited = true;
        open_list.remove(current_index);

        for (auto& connection : current_node.connections)
        {
            int neighbor_index = connection.first;
            PathingNode& neighbor = graph.nodes[neighbor_index];
            float cost_from_current = connection.second;

            if (!neighbor.visited)
            {
                float new_g_cost = current_node.g_cost + cost_from_current;
                float new_f_cost = new_g_cost + neighbor.h_cost;
                if (!neighbor.checked || neighbor.f_cost > new_f_cost)
                {
                    neighbor.g_cost = new_g_cost;
                    neighbor.f_cost = new_f_cost;
                    neighbor.parent = current_index;
                }

                if (!neighbor.checked)
                {
                    neighbor.checked = true;
                    open_list.push_back(neighbor_index);
                }
            }
        }
    }

    if (path_found)
    {
        while (current_index != 0)
        {
            PathingNode& current = graph.nodes[current_index];
            path.push_front(current.position);
            current_index = current.parent;
        }
    }

    return path;
}

} // util
