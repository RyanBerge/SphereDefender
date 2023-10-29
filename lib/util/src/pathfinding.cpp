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

using std::cout, std::cerr, std::endl;

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

PathingGraph CreatePathingGraph(std::vector<sf::FloatRect> obstacles, sf::Vector2f entity_size)
{
    PathingGraph graph;

    // Create two blank nodes for start and finish later
    graph.nodes.push_back(PathingNode{});
    graph.nodes.push_back(PathingNode{});

    float clearance = (std::max(entity_size.x, entity_size.y) / 2) * 1.25f;

    for (auto& rect : obstacles)
    {
        bool collides = false;

        PathingNode upper_left{};
        upper_left.position = sf::Vector2f{rect.left - clearance, rect.top - clearance};
        for (auto& obstacle : obstacles)
        {
            if (util::Contains(obstacle, upper_left.position))
            {
                collides = true;
                break;
            }
        }

        if (!collides)
        {
            graph.nodes.push_back(upper_left);
        }

        PathingNode upper_right{};
        upper_right.position = sf::Vector2f{rect.left + rect.width + clearance, rect.top - clearance};
        for (auto& obstacle : obstacles)
        {
            if (util::Contains(obstacle, upper_right.position))
            {
                collides = true;
                break;
            }
        }

        if (!collides)
        {
            graph.nodes.push_back(upper_right);
        }

        PathingNode lower_left{};
        lower_left.position = sf::Vector2f{rect.left - clearance, rect.top + rect.height + clearance};
        for (auto& obstacle : obstacles)
        {
            if (util::Contains(obstacle, lower_left.position))
            {
                collides = true;
                break;
            }
        }

        if (!collides)
        {
            graph.nodes.push_back(lower_left);
        }

        PathingNode lower_right{};
        lower_right.position = sf::Vector2f{rect.left + rect.width + clearance, rect.top + rect.height + clearance};
        for (auto& obstacle : obstacles)
        {
            if (util::Contains(obstacle, lower_right.position))
            {
                collides = true;
                break;
            }
        }

        if (!collides)
        {
            graph.nodes.push_back(lower_right);
        }
    }

    float sight_width = clearance - 1;

    for (unsigned i = 2; i < graph.nodes.size() - 1; ++i)
    {
        PathingNode& current_node = graph.nodes[i];
        for (unsigned j = i + 1; j < graph.nodes.size(); ++j)
        {
            PathingNode& other_node = graph.nodes[j];

            sf::Vector2f path_vector = other_node.position - current_node.position;
            float length = std::hypot(path_vector.x, path_vector.y);
            sf::Vector2f left_orthogonal{-path_vector.y / length * sight_width, path_vector.x / length * sight_width};
            sf::Vector2f right_orthogonal{path_vector.y / length * sight_width, -path_vector.x / length * sight_width};

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
    }

    return graph;
}

PathingGraph AppendPathingGraph(sf::Vector2f start, sf::Vector2f finish, std::vector<sf::FloatRect> obstacles, sf::FloatRect entity_bounds, const PathingGraph& in_graph)
{
    PathingGraph out_graph = in_graph;

    out_graph.nodes[0].position = start;
    out_graph.nodes[1].position = finish;

    float clearance = (std::max(entity_bounds.width, entity_bounds.height) / 2) * 1.25f;
    float sight_width = clearance - 1;

    for (auto& node : out_graph.nodes)
    {
        node.h_cost = Distance(node.position, out_graph.nodes[1].position);
    }

    for (unsigned i = 0; i < 2; ++i)
    {
        PathingNode& current_node = out_graph.nodes[i];
        for (unsigned j = i + 1; j < out_graph.nodes.size(); ++j)
        {
            PathingNode& other_node = out_graph.nodes[j];

            sf::Vector2f path_vector = other_node.position - current_node.position;
            float length = std::hypot(path_vector.x, path_vector.y);
            LineSegment left_bound;
            LineSegment right_bound;

            if (i == 0)
            {
                // Special case for calculating the line of sight, because we know the position of the entity
                if ((path_vector.x >= 0 && path_vector.y >= 0) || (path_vector.x < 0 && path_vector.y < 0))
                {
                    left_bound.p1 = sf::Vector2f{entity_bounds.left, entity_bounds.top}; // upper left corner
                    right_bound.p1 = sf::Vector2f{entity_bounds.left + entity_bounds.width, entity_bounds.top + entity_bounds.height}; // lower right corner
                }
                else
                {
                    left_bound.p1 = sf::Vector2f{entity_bounds.left, entity_bounds.top + entity_bounds.height}; // lower left corner
                    right_bound.p1 = sf::Vector2f{entity_bounds.left + entity_bounds.width, entity_bounds.top}; // upper right corner
                }

                left_bound.p2 = left_bound.p1 + path_vector;
                right_bound.p2 = right_bound.p1 + path_vector;
            }
            else
            {
                sf::Vector2f left_orthogonal{-path_vector.y / length * sight_width, path_vector.x / length * sight_width};
                sf::Vector2f right_orthogonal{path_vector.y / length * sight_width, -path_vector.x / length * sight_width};

                left_bound = LineSegment{current_node.position + left_orthogonal, other_node.position + left_orthogonal};
                right_bound = LineSegment{current_node.position + right_orthogonal, other_node.position + right_orthogonal};
            }

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

        current_node.h_cost = Distance(current_node.position, out_graph.nodes[1].position);
    }

    return out_graph;
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
            cerr << "Cannot find path\n";
            break;
        }

        // Get node with lowest f-cost from the open list
        current_index = open_list.front();
        for (auto index : open_list)
        {
            if (graph.nodes[index].f_cost < graph.nodes[current_index].f_cost)
            {
                current_index = index;
            }
        }

        if (current_index == 1)
        {
            path_found = true;
            break;
        }

        graph.nodes[current_index].visited = true;
        open_list.remove(current_index);

        for (auto& [neighbor_index, cost_from_current] : graph.nodes[current_index].connections)
        {
            if (!graph.nodes[neighbor_index].visited)
            {
                float new_g_cost = graph.nodes[current_index].g_cost + cost_from_current;
                float new_f_cost = new_g_cost + graph.nodes[neighbor_index].h_cost;
                if (!graph.nodes[neighbor_index].checked || graph.nodes[neighbor_index].f_cost > new_f_cost)
                {
                    graph.nodes[neighbor_index].g_cost = new_g_cost;
                    graph.nodes[neighbor_index].f_cost = new_f_cost;
                    graph.nodes[neighbor_index].parent = current_index;
                }

                if (!graph.nodes[neighbor_index].checked)
                {
                    graph.nodes[neighbor_index].checked = true;
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

std::vector<uint16_t> GetPath(std::vector<DjikstraNode> nodes, uint16_t start, uint16_t finish)
{
    uint16_t current = finish;

    for (auto& node : nodes)
    {
        node.cost = std::numeric_limits<float>::infinity();
        node.visited = false;
    }

    nodes[current].cost = 0;

    while (current != start)
    {
        if (nodes[current].visited)
        {
            cerr << "No possible path from node " << start << " to node " << finish << endl;
            return std::vector<uint16_t>();
        }

        nodes[current].visited = true;

        for (auto& [index, cost] : nodes[current].connections)
        {
            if (nodes[current].cost + cost < nodes[index].cost)
            {
                nodes[index].cost = nodes[current].cost + cost;
                nodes[index].parent = current;
            }
        }

        float minimum = std::numeric_limits<float>::infinity();
        for (unsigned i = 0; i < nodes.size(); ++i)
        {
            DjikstraNode& node = nodes[i];
            if (!node.visited && node.cost < minimum)
            {
                current = i;
                minimum = node.cost;
            }
        }
    }

    std::vector<uint16_t> path;

    while (current != finish)
    {
        path.push_back(current);
        current = nodes[current].parent;
    }
    path.push_back(finish);

    return path;
}

} // util
