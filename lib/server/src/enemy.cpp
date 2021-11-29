/**************************************************************************************************
 *  File:       enemy.cpp
 *  Class:      Enemy
 *
 *  Purpose:    An enemy
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "enemy.h"

namespace server {

namespace {
    const int AGGRO_MAX_THRESHOLD = 800; // pixels
    const int AGGRO_MIN_THRESHOLD = 200;
}

Enemy::Enemy()
{
    // TODO: Enemies should center their position the way players do
    static uint16_t identifier = 0;
    Data.id = identifier++;
    Data.health = 100;
    Data.type = network::EnemyType::SmallDemon;
    Bounds = sf::Vector2f{50, 50};
}

void Enemy::Update(sf::Time elapsed, std::vector<PlayerInfo>& players)
{
    sf::Vector2f target{0, 0};
    double origin_distance = util::Distance(Data.position, target);
    double current_distance = origin_distance;

    for (auto& player : players)
    {
        double new_distance = util::Distance(Data.position, player.Data.position);
        if ((new_distance < current_distance && new_distance < AGGRO_MAX_THRESHOLD) ||
            (target == sf::Vector2f{0, 0} && new_distance < AGGRO_MIN_THRESHOLD))
        {
            current_distance = new_distance;
            target = player.Data.position;
        }
    }

    sf::Vector2f movement_vector = target - Data.position;
    sf::Vector2f velocity{};

    double hyp = std::hypot(movement_vector.x, movement_vector.y);

    if (hyp == 0)
    {
        velocity.x = 0;
        velocity.y = 0;
    }
    else
    {
        velocity.x = (movement_vector.x / hyp) * movement_speed;
        velocity.y = (movement_vector.y / hyp) * movement_speed;
    }

    Data.position += velocity * elapsed.asSeconds();
}

} // server
