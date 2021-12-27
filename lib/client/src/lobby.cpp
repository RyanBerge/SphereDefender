/**************************************************************************************************
 *  File:       lobby.h
 *  Class:      MainMenu
 *
 *  Purpose:    The menu that handles game lobbies
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "lobby.h"
#include "event_handler.h"
#include "game_manager.h"
#include "resources.h"
#include "messaging.h"
#include "settings.h"
#include <iostream>

using std::cout, std::cerr, std::endl;
using network::ClientMessage;

namespace client {

namespace {
    constexpr int LOCAL_PLAYER_FONT_SIZE = 40;
    constexpr int CLASS_FONT_SIZE = 25;
}

Lobby::Lobby()
{
    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    sf::FloatRect bounds;

    start_button.LoadAnimationData("main_menu/start.json");
    bounds = start_button.GetSprite().getGlobalBounds();
    start_button.SetPosition(window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.75 - bounds.height / 2);
    start_button.RegisterLeftMouseUp(std::bind(&Lobby::StartGame, this));

    leave_button.LoadAnimationData("main_menu/leave_lobby.json");
    bounds = leave_button.GetSprite().getGlobalBounds();
    leave_button.SetPosition(window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.9 - bounds.height / 2);
    leave_button.RegisterLeftMouseUp(std::bind(&Lobby::LeaveLobby, this));

    font = resources::FontManager::GetFont("Vera");

    if (font == nullptr)
    {
        std::cerr << "Lobby: There was an error loading the font." << std::endl;
    }

    class_select_right.LoadAnimationData("main_menu/class_select_right.json");
    bounds = class_select_right.GetSprite().getGlobalBounds();
    class_select_right.SetPosition(window_resolution.x * 0.85 - bounds.width / 2, window_resolution.y * 0.15 - bounds.height / 2);
    class_select_right.RegisterLeftMouseUp([this](void){ scrollClassOption(1); });

    class_select_left.LoadAnimationData("main_menu/class_select_left.json");
    bounds = class_select_left.GetSprite().getGlobalBounds();
    class_select_left.SetPosition(class_select_right.GetSprite().getGlobalBounds().left - CLASS_FONT_SIZE * 7, window_resolution.y * 0.15 - bounds.height / 2);
    class_select_left.RegisterLeftMouseUp([this](void){ scrollClassOption(-1); });
}

void Lobby::initializeMenu()
{
    event_id_map[sf::Event::EventType::MouseMoved] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseMoved, std::bind(&Lobby::onMouseMove, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonPressed] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonPressed, std::bind(&Lobby::onMouseDown, this, std::placeholders::_1));
    event_id_map[sf::Event::EventType::MouseButtonReleased] = EventHandler::GetInstance().RegisterCallback(sf::Event::EventType::MouseButtonReleased, std::bind(&Lobby::onMouseUp, this, std::placeholders::_1));

    sf::Text class_option;
    class_option.setFont(*font);
    class_option.setCharacterSize(CLASS_FONT_SIZE);
    class_option.setFillColor(sf::Color::White);

    local_player.class_options[0] = class_option;
    local_player.class_options[1] = class_option;

    sf::FloatRect bounds;
    sf::FloatRect left_button_bounds = class_select_left.GetSprite().getGlobalBounds();
    sf::FloatRect right_button_bounds = class_select_right.GetSprite().getGlobalBounds();
    float center = left_button_bounds.left + left_button_bounds.width + ((right_button_bounds.left - (left_button_bounds.left + left_button_bounds.width)) / 2);

    local_player.class_options[0].setString("Melee");
    bounds = local_player.class_options[0].getGlobalBounds();
    local_player.class_options[0].setOrigin(0, bounds.top - local_player.class_options[0].getPosition().y);
    local_player.class_options[0].setPosition(center - bounds.width / 2, left_button_bounds.top + left_button_bounds.height / 2 - CLASS_FONT_SIZE / 2);

    local_player.class_options[1].setString("Ranged");
    bounds = local_player.class_options[1].getGlobalBounds();
    local_player.class_options[1].setOrigin(0, bounds.top - local_player.class_options[1].getPosition().y);
    local_player.class_options[1].setPosition(center - bounds.width / 2, left_button_bounds.top + left_button_bounds.height / 2 - CLASS_FONT_SIZE / 2);
}

void Lobby::Unload()
{
    for (auto& event : event_id_map)
    {
        EventHandler::GetInstance().UnregisterCallback(event.first, event.second);
    }

    local_player = LobbyPlayer{};
    player_display_list.clear();

    // TODO: Unload UI elements as well?
}

bool Lobby::Create(std::string player_name)
{
    // launch server
    cerr << "Launching server..." << endl;

#if __linux__
    std::system("./Server");
#else
    std::system("start Server.exe");
#endif

    if (!GameManager::GetInstance().ConnectToServer("127.0.0.1"))
    {
        return false;
    }

    ClientMessage::InitLobby(resources::GetServerSocket(), player_name);

    owner = true;
    local_player.data.name = player_name;

    initializeMenu();

    return true;
}

bool Lobby::Join(std::string player_name, std::string ip)
{
    if (!GameManager::GetInstance().ConnectToServer(ip))
    {
        return false;
    }

    ClientMessage::JoinLobby(resources::GetServerSocket(), player_name);

    owner = false;
    local_player.data.name = player_name;

    initializeMenu();

    return true;
}

void Lobby::Draw()
{
    leave_button.Draw();
    if (owner)
    {
        start_button.Draw();
    }

    class_select_left.Draw();
    class_select_right.Draw();

    resources::GetWindow().draw(local_player.display_text);
    resources::GetWindow().draw(local_player.class_options[static_cast<uint8_t>(local_player.data.properties.player_class)]);
    for (auto& display_player : player_display_list)
    {
        resources::GetWindow().draw(display_player.display_text);
        resources::GetWindow().draw(display_player.class_options[static_cast<uint8_t>(display_player.data.properties.player_class)]);
    }
}

void Lobby::StartGame()
{
    if (owner)
    {
        ClientMessage::StartGame(resources::GetServerSocket());
    }

    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::LoadingScreen;

    std::vector<network::PlayerData> data;
    for (auto& player : player_display_list)
    {
        data.push_back(player.data);
    }

    GameManager::GetInstance().Game.Load(local_player.data, data);

    // TODO: Give the Game the player information
    Unload();
}

void Lobby::AssignId(uint16_t id)
{
    local_player.data.id = id;
    local_player.display_text.setFont(*font);
    local_player.display_text.setString(local_player.data.name);
    local_player.display_text.setCharacterSize(LOCAL_PLAYER_FONT_SIZE);

    sf::FloatRect bounds = local_player.display_text.getGlobalBounds();
    local_player.display_text.setOrigin(0, bounds.top - local_player.display_text.getPosition().y);

    updatePlayerPositions();
}

void Lobby::AddPlayer(network::PlayerData player_data)
{
    LobbyPlayer lobby_player;
    lobby_player.data = player_data;
    lobby_player.display_text.setFont(*font);
    lobby_player.display_text.setString(player_data.name);
    lobby_player.display_text.setCharacterSize(LOCAL_PLAYER_FONT_SIZE * 0.8f);

    sf::Text class_option;
    class_option.setFont(*font);
    class_option.setCharacterSize(CLASS_FONT_SIZE);
    class_option.setFillColor(sf::Color::White);

    lobby_player.class_options[0] = class_option;
    lobby_player.class_options[1] = class_option;

    lobby_player.class_options[0].setString("Melee");
    sf::FloatRect bounds = lobby_player.class_options[0].getGlobalBounds();
    lobby_player.class_options[0].setOrigin(0, bounds.top - lobby_player.class_options[0].getPosition().y);

    lobby_player.class_options[1].setString("Ranged");
    bounds = lobby_player.class_options[1].getGlobalBounds();
    lobby_player.class_options[1].setOrigin(0, bounds.top - lobby_player.class_options[1].getPosition().y);

    player_display_list.push_back(lobby_player);

    updatePlayerPositions();
}

void Lobby::RemovePlayer(uint16_t player_id)
{
    for (auto iterator = player_display_list.begin(); iterator != player_display_list.end(); ++iterator)
    {
        if (iterator->data.id == player_id)
        {
            player_display_list.erase(iterator);
            break;
        }
    }

    updatePlayerPositions();
}

void Lobby::SetPlayerProperties(uint16_t player_id, network::PlayerProperties properties)
{
    for (auto& player : player_display_list)
    {
        if (player.data.id == player_id)
        {
            player.data.properties = properties;
            break;
        }
    }
}

void Lobby::LeaveLobby()
{
    Unload();
    ClientMessage::LeaveGame(resources::GetServerSocket());
    GameManager::GetInstance().DisconnectFromServer();
    GameManager::GetInstance().MainMenu.CurrentMenu = MainMenu::MenuType::Main;
}

void Lobby::updatePlayerPositions()
{
    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    int offset = 0;

    sf::Vector2f local_class_position = local_player.class_options[0].getPosition();
    sf::FloatRect bounds = local_player.display_text.getGlobalBounds();
    sf::FloatRect reference_bounds = local_player.class_options[0].getGlobalBounds();

    float base_y = local_class_position.y - bounds.height / 2 + reference_bounds.height / 2;

    local_player.display_text.setPosition(sf::Vector2f{window_resolution.x * 0.1f, base_y});
    offset += local_player.display_text.getCharacterSize() * 1.2;

    for (auto& player : player_display_list)
    {
        player.display_text.setPosition(sf::Vector2f(window_resolution.x * 0.1f, base_y + offset));
        for (auto& option : player.class_options)
        {
            option.setPosition(local_player.class_options[0].getPosition().x, base_y + offset);
        }
        offset += player.display_text.getCharacterSize() * 1.2;
    }
}

void Lobby::scrollClassOption(int displacement)
{
    uint8_t option = static_cast<uint8_t>(local_player.data.properties.player_class);
    option = (option + displacement) % local_player.class_options.size();
    local_player.data.properties.player_class = static_cast<network::PlayerClass>(option);
    if (local_player.data.properties.player_class == network::PlayerClass::Melee)
    {
        local_player.data.properties.weapon_type = definitions::WeaponType::Sword;
    }
    else
    {
        local_player.data.properties.weapon_type = definitions::WeaponType::BurstGun;
    }

    ClientMessage::ChangePlayerProperty(resources::GetServerSocket(), local_player.data.properties);
}

void Lobby::onMouseMove(sf::Event event)
{
    leave_button.UpdateMousePosition(event.mouseMove);
    start_button.UpdateMousePosition(event.mouseMove);
    class_select_left.UpdateMousePosition(event.mouseMove);
    class_select_right.UpdateMousePosition(event.mouseMove);
}

void Lobby::onMouseDown(sf::Event event)
{
    leave_button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
    start_button.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
    class_select_left.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
    class_select_right.UpdateMouseState(event.mouseButton, CursorButton::State::Down);
}

void Lobby::onMouseUp(sf::Event event)
{
    leave_button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
    start_button.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
    class_select_left.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
    class_select_right.UpdateMouseState(event.mouseButton, CursorButton::State::Up);
}

} // client
