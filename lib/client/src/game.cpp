/**************************************************************************************************
 *  File:       game.h
 *  Class:      Game
 *
 *  Purpose:    The top-level controller for everything in-game
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "game.h"
#include "resources.h"
#include "event_handler.h"
#include "settings.h"
#include "messaging.h"
#include "debug_overrides.h"
#include <thread>
#include <iostream>
#include <cmath>
#include "SFML/System/Sleep.hpp"

using std::cout, std::cerr, std::endl;
using network::ClientMessage, network::ServerMessage;

namespace client {
namespace {
    constexpr int FADE_TIME = 3;
}

Game::Game()
{
    resources::GetWorldView().setSize(Settings::GetInstance().WindowResolution);
    black_overlay.setSize(Settings::GetInstance().WindowResolution);
    black_overlay.setFillColor(sf::Color{0, 0, 0, 0});
}

void Game::Update(sf::Time elapsed)
{
    if (loaded)
    {
        gui.Update(elapsed);

        if (local_player.ActionsDisabled() && !gui.DisableActions() && !inCutscene())
        {
            local_player.SetActionsEnabled(true);
        }

        local_player.Update(elapsed);
        for (auto& avatar : avatars)
        {
            avatar.second.Update(elapsed);
        }

        for (auto& enemy : enemies)
        {
            enemy.second.Update(elapsed);
        }

        region_map.Update(elapsed);

        gui.MarkInteractables(local_player.GetPosition(), region_map.GetInteractablePositions());

        if (current_zoom != target_zoom)
        {
            current_zoom += zoom_speed * elapsed.asSeconds();
            zoom_speed = (target_zoom - current_zoom) * 4;

            if (std::abs(current_zoom - target_zoom) < 0.001)
            {
                current_zoom = target_zoom;
            }

            resources::GetWorldView().setSize(Settings::GetInstance().WindowResolution * current_zoom);
        }

        if (leaving_region)
        {
            handleLeavingRegion();
        }
        else if (entering_region)
        {
            handleEnteringRegion();
        }
        else
        {
            sf::View& world_view = resources::GetWorldView();
            world_view.setCenter(local_player.GetPosition());

            sf::FloatRect view_bounds;
            view_bounds.left = world_view.getCenter().x - world_view.getSize().x / 2;
            view_bounds.top = world_view.getCenter().y - world_view.getSize().y / 2;
            view_bounds.width = world_view.getSize().x;
            view_bounds.height = world_view.getSize().y;

            float left_limit = region_map.Bounds.left - local_player.Avatar.GetCollisionDimensions().x - (15 * current_zoom);
            float right_limit = region_map.Bounds.left + region_map.Bounds.width + local_player.Avatar.GetCollisionDimensions().x + (15 * current_zoom);
            float top_limit = region_map.Bounds.top - local_player.Avatar.GetCollisionDimensions().y - (15 * current_zoom);
            float bottom_limit = region_map.Bounds.top + region_map.Bounds.height + local_player.Avatar.GetCollisionDimensions().y + (15 * current_zoom);

            if (view_bounds.left < left_limit)
            {
                world_view.move(left_limit - view_bounds.left, 0);
            }
            else if (view_bounds.left + view_bounds.width > right_limit)
            {
                world_view.move(right_limit - (view_bounds.left + view_bounds.width), 0);
            }

            if (view_bounds.top < top_limit)
            {
                world_view.move(0,top_limit - view_bounds.top);
            }
            else if (view_bounds.top + view_bounds.height > bottom_limit)
            {
                world_view.move(0,bottom_limit - (view_bounds.top + view_bounds.height));
            }
        }
    }
}

void Game::Draw()
{
    if (loaded)
    {
        sf::View old_view = resources::GetWindow().getView();
        resources::GetWorldView().setViewport(old_view.getViewport());
        resources::GetWindow().setView(resources::GetWorldView());

        region_map.Draw();

        for (auto& projectile : projectiles)
        {
            resources::GetWindow().draw(projectile);
        }

        for (auto& enemy : enemies)
        {
            enemy.second.Draw();
        }

        if (!inCutscene())
        {
            local_player.Draw();
            for (auto& avatar : avatars)
            {
                avatar.second.Draw();
            }
        }

        if (display_debug_path)
        {
            for (auto& node : debug_graph_nodes)
            {
                resources::GetWindow().draw(node);
            }

            for (auto& node : debug_path_nodes)
            {
                resources::GetWindow().draw(node);
            }
        }

        resources::GetWindow().setView(old_view);

        gui.Draw();

        if (menu_open)
        {
            Settings::GetInstance().Draw();
        }

        if (inCutscene())
        {
            resources::GetWindow().draw(black_overlay);
        }
    }
}

void Game::Load(network::PlayerData local, std::vector<network::PlayerData> other_players)
{
    loaded = false;
    zone_loaded = false;
    std::thread loading_thread(&Game::asyncLoad, this, local, other_players);
    loading_thread.detach();
}

void Game::asyncLoad(network::PlayerData local, std::vector<network::PlayerData> other_players)
{
    cout << "Async load started..." << endl;

    event_id_map[sf::Event::EventType::MouseMoved] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&Game::onMouseMove, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Game::onMouseDown, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Game::onMouseUp, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::TextEntered] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::TextEntered, std::bind(&Game::onTextEntered, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::KeyPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyPressed, std::bind(&Game::onKeyPressed, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::KeyReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::KeyReleased, std::bind(&Game::onKeyReleased, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseWheelScrolled] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseWheelScrolled, std::bind(&Game::onMouseWheel, this, std::placeholders::_1));

    menu_open = false;
    zoom_factor = 0;
    current_zoom = 1;
    target_zoom = current_zoom;
    zoom_speed = 0;

    avatars.clear();
    enemies.clear();

    local_player = Player();
    local_player.Load(local);
    for (auto& player : other_players)
    {
        avatars[player.id] = Avatar(sf::Color(180, 115, 150), player);
    }

    std::unique_lock<std::mutex> lock(loading_mutex);
    zone_loaded_condition.wait(lock, [this](){ return isZoneLoaded(); });

    // In-place reconstruction to avoid deleted assignment operator
    gui.~Gui();
    new(&gui)Gui();
    gui.Load(current_zone);

    region_map = RegionMap();
    region_map.Load(current_zone.regions[debug::StartingRegion.value].type);

    resources::GetWorldView() = sf::View(sf::FloatRect(0, 0, Settings::GetInstance().WindowResolution.x, Settings::GetInstance().WindowResolution.y));

    sf::sleep(sf::seconds(1));

    loaded = true;
    cout << "Async load finished." << endl;

    ClientMessage::LoadingComplete(resources::GetServerSocket());
}

bool Game::isZoneLoaded()
{
    return zone_loaded;
}

void Game::Unload()
{
    for (auto& event : event_id_map)
    {
        EventHandler::GetInstance().UnregisterCallback(event.first, event.second);
    }

    // TODO: These load/unload functions probably need to be protected by mutexes in case the host leaves when the game is loading
    region_map.Unload();
    gui.Unload();
    local_player.Unload();
}

void Game::Start(sf::Vector2f spawn_position)
{
    local_player.SetPosition(spawn_position);
    resources::GetWorldView().setCenter(spawn_position);
}

int Game::GetPlayerCount()
{
    return avatars.size() + 1;
}

std::vector<uint16_t> Game::GetPlayerIds()
{
    std::vector<uint16_t> ids;
    ids.push_back(local_player.Avatar.Data.id);
    for (auto& [id, avatar] : avatars)
    {
        ids.push_back(id);
    }

    return ids;
}

std::string Game::GetPlayerName(uint16_t player_id)
{
    if (player_id == local_player.Avatar.Data.id)
    {
        return local_player.Avatar.Data.name;
    }

    return avatars[player_id].Data.name;
}

void Game::SetZone(definitions::Zone zone)
{
    current_zone = zone;
    zone_loaded = true;
    zone_loaded_condition.notify_all();
}

void Game::UpdatePlayerStates(std::vector<network::PlayerData> player_list)
{
    for (auto& player : player_list)
    {
        if (player.id == local_player.Avatar.Data.id)
        {
            local_player.SetPosition(player.position);
            local_player.Avatar.UpdateHealth(player.health);
            gui.UpdateHealth(player.health);
        }
        else
        {
            avatars[player.id].SetPosition(player.position);
            avatars[player.id].UpdateHealth(player.health);
        }
    }
}

void Game::AddEnemy(uint16_t enemy_id, definitions::EntityType type)
{
    if (enemies.find(enemy_id) == enemies.end())
    {
        enemies.emplace(enemy_id, Enemy(type));
    }
}

void Game::UpdateEnemies(std::vector<network::EnemyData> enemy_list)
{
    for (auto& enemy : enemy_list)
    {
        if (enemies.find(enemy.id) == enemies.end())
        {
            //enemies[enemy.id] = Enemy(enemy.type);
            enemies.emplace(enemy.id, Enemy(enemy.type));
        }
        else
        {
            enemies[enemy.id].UpdateData(enemy);
        }
    }
}

void Game::UpdateProjectiles(std::vector<network::ProjectileData> projectile_list)
{
    projectiles.clear();
    for (auto& data : projectile_list)
    {
        sf::RectangleShape projectile;
        projectile.setPosition(data.position);
        projectile.setFillColor(sf::Color::Black);
        projectile.setSize(sf::Vector2f{4, 4});
        projectiles.push_back(projectile);
    }
}

void Game::UpdateBattery(float battery_level)
{
    gui.UpdateBatteryBar(battery_level);
}

void Game::SetPaused(bool paused, network::GuiType gui_type)
{
    switch (gui_type)
    {
        case network::GuiType::Overmap:
        {
            gui.SetOvermapDisplay(paused);
        }
        break;
        case network::GuiType::MenuEvent:
        {
            //gui.DisplayMenuEvent(definitions::GetRegionDefinition(region_map.RegionType).events[0]);
        }
        break;
    }

    local_player.SetActionsEnabled(!paused);
    IsPaused = paused;
}

void Game::SetPlayerActionsEnabled(bool enable)
{
    local_player.SetActionsEnabled(enable);
}

void Game::StartAction(uint16_t player_id, network::PlayerAction action)
{
    switch (action.type)
    {
        case network::PlayerActionType::Attack:
        {
            if (local_player.Avatar.Data.id == player_id)
            {
                local_player.Avatar.StartAttack(action.action_angle);
            }
            else
            {
                avatars[player_id].StartAttack(action.action_angle);
            }
        }
        break;
        case network::PlayerActionType::Stunned:
        {
            if (local_player.Avatar.Data.id == player_id)
            {
                local_player.Avatar.SetStunned(action.duration);
            }
            else
            {
                avatars[player_id].SetStunned(action.duration);
            }
        }
    }
}

void Game::ChangeEnemyAnimation(uint16_t enemy_id, definitions::AnimationName animation_name, util::Direction direction)
{
    enemies[enemy_id].ChangeAnimation(animation_name, direction);
}

void Game::RemovePlayer(uint16_t player_id)
{
    // TODO: Thread-safety
    cout << avatars[player_id].Data.name << " disconnected." << endl;
    avatars.erase(player_id);
}

void Game::ChangeItem(definitions::ItemType item)
{
    gui.ChangeItem(item);
}

void Game::ChangeRegion(uint16_t region_id)
{
    gui.SetEnabled(false);
    target_zoom = 1.2;
    local_player.SetActionsEnabled(false);
    region_map.LeaveRegion();

    next_region = region_id;

    leaving_region = true;
    leaving_region_state = LeavingRegionState::Start;
}

void Game::EnterRegion(sf::Vector2f spawn_position)
{
    leaving_region = false;
    entering_region = true;
    entering_region_state = EnteringRegionState::Start;
    local_player.SetPosition(spawn_position);
}

void Game::SetMenuEvent(uint16_t event_id)
{
    if (region_map.RegionType != definitions::RegionType::MenuEvent)
    {
        cerr << "Server tried to set event id for a region with no event." << endl;
        return;
    }

    gui.DisplayMenuEvent(definitions::GetMenuEventById(event_id), 0);
    SetPlayerActionsEnabled(false);
}

void Game::AdvanceMenuEvent(uint16_t advance_value, bool finish)
{
    gui.AdvanceMenuEvent(advance_value, finish);
}

void Game::UpdateStash(std::array<definitions::ItemType, 24> items)
{
    gui.UpdateStash(items);
}

void Game::DisplayGatherPlayers(uint16_t player_id, bool start)
{
    gui.DisplayGatherPlayers(player_id, start);
}

void Game::DisplayVote(uint16_t player_id, uint8_t vote, bool confirmed)
{
    gui.DisplayVote(player_id, vote, confirmed);
}

void Game::DisplayDebugPath(std::vector<sf::Vector2f> graph, std::vector<sf::Vector2f> path)
{
    display_debug_path = true;
    debug_graph_nodes.clear();
    debug_path_nodes.clear();

    //for (auto& node : graph)
    for (unsigned i = 0; i < graph.size(); ++i)
    {
        sf::Text number;
        number.setFont(*resources::FontManager::GetFont("Vera"));
        number.setFillColor(sf::Color::Red);
        number.setOutlineColor(sf::Color::Black);
        number.setOutlineThickness(2);
        number.setCharacterSize(16);
        number.setString(std::to_string(i));
        //number.setRadius(4);
        number.setPosition(graph[i]);
        number.setOrigin(number.getLocalBounds().left + number.getLocalBounds().width / 2, number.getLocalBounds().top + number.getLocalBounds().height / 2);
        debug_graph_nodes.push_back(number);
    }

    //debug_graph_nodes[13].setFillColor(sf::Color::Yellow);

    for (auto& node : path)
    {
        sf::CircleShape dot;
        dot.setFillColor(sf::Color::Blue);
        dot.setRadius(5);
        dot.setPosition(node);
        dot.setOrigin(dot.getLocalBounds().left + dot.getLocalBounds().width / 2, dot.getLocalBounds().top + dot.getLocalBounds().height / 2);
        debug_path_nodes.push_back(dot);
    }
}

void Game::updateScroll(sf::Time elapsed)
{
    sf::Vector2f mouse_coords = resources::GetWindow().mapPixelToCoords(sf::Mouse::getPosition() - resources::GetWindow().getPosition(), gui.GuiView);

    sf::Vector2i scroll_factor{0, 0};
    sf::Vector2f resolution = Settings::GetInstance().WindowResolution;

    int edge_threshold = 15;

    if (mouse_coords.x < edge_threshold)
    {
        scroll_factor.x = -1;
    }
    else if (mouse_coords.x > resolution.x - edge_threshold)
    {
        scroll_factor.x = 1;
    }

    if (mouse_coords.y < edge_threshold)
    {
        scroll_factor.y = -1;
    }
    else if (mouse_coords.y > resolution.y - edge_threshold)
    {
        scroll_factor.y = 1;
    }

    resources::GetWorldView().move(sf::Vector2f(scroll_factor * Settings::GetInstance().ScrollSpeed) * current_zoom * elapsed.asSeconds());
}

void Game::handleLeavingRegion()
{
    static uint8_t overlay_opacity = 0;
    static sf::Clock fade_timer;

    switch (leaving_region_state)
    {
        case LeavingRegionState::Start:
        {
            overlay_opacity = 0;
            leaving_region_state = LeavingRegionState::Moving;
            [[fallthrough]];
        }
        case LeavingRegionState::Moving:
        {
            resources::GetWorldView().setCenter(region_map.GetConvoyPosition().x + 200, region_map.GetConvoyPosition().y);
            if (region_map.GetConvoyPosition().y < definitions::GetRegionDefinition(region_map.RegionType).convoy.Position.y - Settings::GetInstance().WindowResolution.y * 2)
            {
                fade_timer.restart();
                leaving_region_state = LeavingRegionState::Fading;
            }
        }
        break;
        case LeavingRegionState::Fading:
        {
            overlay_opacity = std::floor(fade_timer.getElapsedTime().asSeconds() / FADE_TIME * 255);
            black_overlay.setFillColor(sf::Color{0, 0, 0, overlay_opacity});
            if (fade_timer.getElapsedTime().asSeconds() > FADE_TIME)
            {
                leaving_region_state = LeavingRegionState::Loading;
            }
        }
        break;
        case LeavingRegionState::Loading:
        {
            region_map = RegionMap();

            for (auto& node : current_zone.regions)
            {
                if (node.id == next_region)
                {
                    region_map.Load(node.type);
                    break;
                }
            }

            gui.ChangeRegion(next_region);
            ClientMessage::LoadingComplete(resources::GetServerSocket());
            leaving_region_state = LeavingRegionState::Waiting;
            [[fallthrough]];
        }
        case LeavingRegionState::Waiting: { }
        break;
    }
}

void Game::handleEnteringRegion()
{
    static uint8_t overlay_opacity = 255;
    static sf::Clock fade_timer;

    resources::GetWorldView().setCenter(region_map.GetConvoyPosition().x + 200, region_map.GetConvoyPosition().y);

    switch (entering_region_state)
    {
        case EnteringRegionState::Start:
        {
            region_map.EnterRegion();
            enemies.clear();
            entering_region_state = EnteringRegionState::Fading;
            fade_timer.restart();
            [[fallthrough]];
        }
        case EnteringRegionState::Fading:
        {
            overlay_opacity = 255 - std::floor(fade_timer.getElapsedTime().asSeconds() / FADE_TIME * 255);
            black_overlay.setFillColor(sf::Color{0, 0, 0, overlay_opacity});
            if (fade_timer.getElapsedTime().asSeconds() > FADE_TIME)
            {
                entering_region_state = EnteringRegionState::Moving;
            }
        }
        break;
        case EnteringRegionState::Moving:
        {
            if (region_map.GetConvoyPosition().y == definitions::GetRegionDefinition(region_map.RegionType).convoy.Position.y)
            {
                gui.SetEnabled(true);
                target_zoom = 1;
                entering_region = false;

                if (region_map.RegionType != definitions::RegionType::MenuEvent)
                {
                    local_player.SetActionsEnabled(true);
                }
            }
        }
        break;
    }
}

bool Game::inCutscene()
{
    return entering_region || leaving_region;
}

// TODO: Ensure that these are only updated for elements that can be controlled at that time
void Game::onMouseMove(sf::Event event)
{
    if (loaded)
    {
        gui.OnMouseMove(event.mouseMove);
        local_player.OnMouseMove(event.mouseMove);
    }
}

void Game::onMouseDown(sf::Event event)
{
    if (loaded)
    {
        gui.OnMouseDown(event.mouseButton);
        local_player.OnMouseDown(event.mouseButton);
    }
}

void Game::onMouseUp(sf::Event event)
{
    if (loaded)
    {
        gui.OnMouseUp(event.mouseButton);
        local_player.OnMouseUp(event.mouseButton);
    }
}

void Game::onTextEntered(sf::Event event)
{
    if (loaded)
    {
        gui.OnTextEntered(event.text);
        local_player.OnTextEntered(event.text);
    }
}

void Game::onKeyPressed(sf::Event event)
{
    if (loaded)
    {
        if (!menu_open) {
            local_player.OnKeyPressed(event.key);
        }

        if (event.key.code == Settings::GetInstance().Bindings.Escape)
        {
            gui.EscapePressed();
        }

        if (gui.Available() && event.key.code == Settings::GetInstance().Bindings.Interact)
        {
            RegionMap::Interaction interaction = region_map.Interact(local_player.GetPosition());
            switch (interaction.type)
            {
                case RegionMap::InteractionType::None: { }
                break;
                case RegionMap::InteractionType::NpcDialog:
                {
                    gui.DisplayDialog(interaction.npc_name, interaction.dialog);
                    local_player.SetActionsEnabled(false);
                }
                break;
                case RegionMap::InteractionType::ConvoyConsole:
                {
                    ClientMessage::Console(resources::GetServerSocket(), true);
                    local_player.SetActionsEnabled(false);
                }
                break;
                case RegionMap::InteractionType::ConvoyStash:
                {
                    gui.DisplayStash();
                    local_player.SetActionsEnabled(false);
                }
                break;
            }
        }

        if (gui.Available() && event.key.code == Settings::GetInstance().Bindings.Item)
        {
            ClientMessage::UseItem(resources::GetServerSocket());
        }
    }
}

void Game::onKeyReleased(sf::Event event)
{
    if (loaded)
    {
        local_player.OnKeyReleased(event.key);
    }
}

void Game::onMouseWheel(sf::Event event)
{
    if (loaded && !inCutscene())
    {
        Settings& settings = Settings::GetInstance();

        zoom_factor -= event.mouseWheelScroll.delta;
        if (zoom_factor < settings.MinZoomFactor)
        {
            zoom_factor = settings.MinZoomFactor;
        }
        else if (zoom_factor > settings.MaxZoomFactor)
        {
            zoom_factor = settings.MaxZoomFactor;
        }

        target_zoom = 1 + static_cast<float>(zoom_factor) / 10;
        zoom_speed = target_zoom - current_zoom;
    }
}

} // client
