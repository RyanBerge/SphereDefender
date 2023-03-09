/**************************************************************************************************
 *  File:       gui.cpp
 *  Class:      Gui
 *
 *  Purpose:    Represents the in-game GUI
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include "gui.h"
#include "settings.h"
#include "resources.h"
#include "game_manager.h"
#include "game_math.h"
#include <iostream>
#include <string>
#include <set>

using std::cout, std::cerr, std::endl;
using network::ClientMessage;

namespace client {
namespace {
    constexpr int INTERACTION_DISTANCE = 75;
}

Gui::Gui() { }

void Gui::Draw()
{
    if (enabled)
    {
        sf::View old_view = resources::GetWindow().getView();

        resources::GetWorldView().setViewport(old_view.getViewport());
        resources::GetWindow().setView(resources::GetWorldView());

        if (!overmap.IsActive())
        {
            interaction_marker.Draw();
        }

        GuiView.setViewport(resources::GetWorldView().getViewport());
        resources::GetWindow().setView(GuiView);

        ui_frame.Draw();
        menu_button.Draw();

        if (!InDialog)
        {
            resources::GetWindow().draw(healthbar);
            healthbar_frame.Draw();
            inventory_item.Draw();
        }

        resources::GetWindow().draw(battery_bar);
        resources::GetWindow().draw(battery_bar_frame);

        if (Health == 0)
        {
            resources::GetWindow().draw(death_tint);
            resources::GetWindow().draw(death_text);
        }

        if (InDialog)
        {
            resources::GetWindow().draw(dialog_frame);
            for (auto& text : dialog_text)
            {
                resources::GetWindow().draw(text);
            }
            resources::GetWindow().draw(dialog_source_text);
            resources::GetWindow().draw(dialog_prompt_text);
        }

        if (overmap.IsActive())
        {
            overmap.Draw();
            for (auto& [id, vote] : vote_indicators)
            {
                vote.indicator.Draw();
            }
        }

        if (stash.Active)
        {
            stash.Draw();
        }

        if (in_event)
        {
            resources::GetWindow().draw(event_background);
            event_prompt.Draw();
            for (auto& option : event_options)
            {
                option.Draw();
            }

            for (auto& [id, vote] : vote_indicators)
            {
                vote.indicator.Draw();
            }

            if (event_options.size() > 0)
            {
                event_vote_confirm_button.Draw();
            }
        }

        if (gathering)
        {
            resources::GetWindow().draw(gather_text);
        }

        if (InMenus)
        {
            menu.Draw();
            resume_button.Draw();
            save_button.Draw();
            settings_button.Draw();
            exit_button.Draw();
        }

        resources::GetWindow().setView(old_view);
    }
}

void Gui::Load(definitions::Zone zone)
{
    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    font = resources::FontManager::GetFont("Vera");
    sf::FloatRect bounds;

    ui_frame.LoadAnimationData("gui/frame.json");
    ui_frame.SetPosition(0, 0);

    menu_button.LoadAnimationData("gui/menu_button.json");
    menu_button.SetPosition(901, 7);
    menu_button.RegisterLeftMouseUp([this](void){ DisplayMenu(); });

    healthbar.setSize(sf::Vector2f{16, 100});
    healthbar.setOrigin(sf::Vector2f{0, healthbar.getSize().y});
    healthbar.setPosition(sf::Vector2f{window_resolution.x * 0.05f, window_resolution.y * 0.95f});
    healthbar.setFillColor(sf::Color::Green);

    healthbar_frame.LoadAnimationData("gui/healthbar.json");
    healthbar_frame.GetSprite().setOrigin(sf::Vector2f{0, healthbar.getSize().y});
    healthbar_frame.SetPosition(sf::Vector2f{window_resolution.x * 0.05f - 2, window_resolution.y * 0.95f});

    inventory_item.LoadAnimationData("gui/inventory_item.json");
    inventory_item.SetPosition(sf::Vector2f{window_resolution.x * 0.15f, window_resolution.y * 0.88f});
    inventory_item.SetAnimation("Medpack");

    battery_bar_frame.setSize(sf::Vector2f{800, 20});
    battery_bar_frame.setOrigin(sf::Vector2f{0, 10});
    battery_bar_frame.setPosition(sf::Vector2f{40, 40});
    battery_bar_frame.setOutlineColor(sf::Color::Black);
    battery_bar_frame.setOutlineThickness(2);
    battery_bar_frame.setFillColor(sf::Color::Transparent);

    battery_bar.setSize(sf::Vector2f{800, 20});
    battery_bar.setOrigin(sf::Vector2f{0, 10});
    battery_bar.setPosition(sf::Vector2f{40, 40});
    battery_bar.setFillColor(sf::Color{220, 255, 128});
    battery_bar.setScale(sf::Vector2f{0, 1});

    dialog_frame.setSize(sf::Vector2f{window_resolution.x * 0.95f, window_resolution.y * 0.3f});
    dialog_frame.setPosition(sf::Vector2f{window_resolution.x * 0.025f, window_resolution.y * 0.65f});
    dialog_frame.setFillColor(sf::Color{125, 125, 125, 200});
    dialog_frame.setOutlineColor(sf::Color::Black);
    dialog_frame.setOutlineThickness(3);

    dialog_source_text.setFont(*font);
    dialog_source_text.setCharacterSize((Settings::GetInstance().WindowResolution.y * 0.3f) / 8);
    dialog_source_text.setPosition(sf::Vector2f{dialog_frame.getPosition().x + dialog_source_text.getCharacterSize() * 0.5f, dialog_frame.getPosition().y - dialog_source_text.getCharacterSize() * 1.5f});
    dialog_source_text.setFillColor(sf::Color::Black);

    bounds = dialog_frame.getGlobalBounds();
    dialog_prompt_text.setString("Click to continue.");
    dialog_prompt_text.setFont(*font);
    dialog_prompt_text.setCharacterSize((Settings::GetInstance().WindowResolution.y * 0.3f) / 10);
    dialog_prompt_text.setPosition(sf::Vector2f{bounds.left + bounds.width - dialog_prompt_text.getGlobalBounds().width * 1.1f, bounds.top + bounds.height - dialog_prompt_text.getGlobalBounds().height * 3.0f});
    dialog_prompt_text.setFillColor(sf::Color::Black);

    death_text.setFont(*font);
    death_text.setString("You are dead.");
    death_text.setCharacterSize(60);
    bounds = death_text.getGlobalBounds();
    death_text.setPosition(sf::Vector2f{window_resolution.x / 2 - bounds.width / 2, window_resolution.y / 2 - bounds.height / 2});
    death_text.setFillColor(sf::Color::Black);
    death_text.setOutlineColor(sf::Color::White);
    death_text.setOutlineThickness(1);

    death_tint.setSize(window_resolution);
    death_tint.setPosition(0, 0);
    death_tint.setFillColor(sf::Color{100, 100, 100, 230});

    interaction_marker.LoadAnimationData("gui/interaction_marker.json");

    gather_text.setFont(*font);
    gather_text.setCharacterSize(25);
    gather_text.setFillColor(sf::Color::White);
    gather_text.setOutlineColor(sf::Color::Black);
    gather_text.setOutlineThickness(1);

    menu.LoadAnimationData("gui/menu.json");
    bounds = menu.GetSprite().getGlobalBounds();
    menu.SetPosition(sf::Vector2f{window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.125f});

    sf::FloatRect reference_bounds = menu.GetSprite().getGlobalBounds();

    // TODO: These callbacks are not instance-safe and will cause crashes
    resume_button.LoadAnimationData("gui/resume.json");
    bounds = resume_button.GetSprite().getGlobalBounds();
    resume_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.2 - bounds.height / 2);
    resume_button.RegisterLeftMouseUp([this](void){ InMenus = false; });

    save_button.LoadAnimationData("gui/save.json");
    bounds = save_button.GetSprite().getGlobalBounds();
    save_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.35 - bounds.height / 2);

    settings_button.LoadAnimationData("gui/settings.json");
    bounds = settings_button.GetSprite().getGlobalBounds();
    settings_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.5 - bounds.height / 2);

    exit_button.LoadAnimationData("gui/exit.json");
    bounds = exit_button.GetSprite().getGlobalBounds();
    exit_button.SetPosition(reference_bounds.left + reference_bounds.width / 2 - bounds.width / 2, reference_bounds.top + reference_bounds.height * 0.8 - bounds.height / 2);
    exit_button.RegisterLeftMouseUp(std::bind(&Gui::exitGame, this));

    event_background.setFillColor(sf::Color{130, 150, 255, 255});
    event_background.setSize(sf::Vector2f{window_resolution.x * 0.8f, window_resolution.y * 0.8f});
    event_background.setOrigin(sf::Vector2f{event_background.getSize().x / 2, event_background.getSize().y / 2});
    event_background.setPosition(sf::Vector2f{window_resolution.x / 2, window_resolution.y /2});

    for (auto& id : GameManager::GetInstance().Game.GetPlayerIds())
    {
        vote_indicators[id].indicator = Spritesheet("gui/vote_indicator.json");
        vote_indicators[id].indicator.SetVisible(false);
        vote_indicators[id].vote = -1;
    }

    sf::FloatRect background_bounds = event_background.getGlobalBounds();
    event_vote_confirm_button.LoadAnimationData("gui/overmap_confirm.json");
    float x = background_bounds.left + background_bounds.width - (event_vote_confirm_button.GetGlobalBounds().width * 1.2f);
    float y = background_bounds.top + background_bounds.height - (event_vote_confirm_button.GetGlobalBounds().height * 1.2f);
    event_vote_confirm_button.SetPosition(x, y);
    event_vote_confirm_button.RegisterOnToggle(std::bind(&Gui::onConfirmClick, this, std::placeholders::_1));
    event_vote_confirm_button.SetEnabled(false);

    GuiView = sf::View(sf::FloatRect(0, 0, window_resolution.x, window_resolution.y));

    InMenus = false;
    InDialog = false;
    UpdateHealth(100);
    overmap.Load(zone);
    overmap.SetRegion(definitions::STARTING_REGION);

    is_loaded = true;
}

void Gui::Unload()
{

}

bool Gui::IsLoaded()
{
    return is_loaded;
}

void Gui::SetEnabled(bool new_enabled)
{
    enabled = new_enabled;

    InMenus = false;
    InDialog = false;
    overmap.SetActive(false);
    stash.Active = false;
}

void Gui::UpdateHealth(uint8_t value)
{
    Health = value;
    healthbar.setScale(sf::Vector2f{1, static_cast<float>(Health) / 100});
}

void Gui::UpdateBatteryBar(float battery_level)
{
    battery_bar.setScale(sf::Vector2f{battery_level / 1000, 1});
}

void Gui::UpdateStash(std::array<definitions::ItemType, 24> items)
{
    stash.UpdateItems(items);
}

void Gui::ChangeItem(definitions::ItemType item)
{
    switch (item)
    {
        case definitions::ItemType::None:
        {
            inventory_item.SetAnimation("None");
        }
        break;
        case definitions::ItemType::Medpack:
        {
            inventory_item.SetAnimation("Medpack");
        }
        break;
    }
}

void Gui::ChangeRegion(uint16_t region_id)
{
    overmap.SetRegion(region_id);
}

void Gui::MarkInteractables(sf::Vector2f player_position, std::vector<sf::FloatRect> bounds_list)
{
    double distance = std::numeric_limits<double>::infinity();
    unsigned index;

    for (unsigned i = 0; i < bounds_list.size(); ++i)
    {
        sf::Vector2f position = sf::Vector2f{bounds_list[i].left + bounds_list[i].width / 2, bounds_list[i].top + bounds_list[i].height / 2};
        double temp_distance = util::Distance(player_position, position);
        if (temp_distance < distance)
        {
            distance = temp_distance;
            index = i;
        }
    }

    if (distance < INTERACTION_DISTANCE)
    {
        interaction_marker.SetVisible(true);
        interaction_marker.SetPosition(bounds_list[index].left + bounds_list[index].width / 2, bounds_list[index].top - 5);
    }
    else
    {
        interaction_marker.SetVisible(false);
    }
}

bool Gui::Available()
{
    return !(InMenus || InDialog || overmap.IsActive());
}

bool Gui::DisableActions()
{
    return InDialog || overmap.IsActive() || stash.Active;
}

void Gui::EscapePressed()
{
    if (overmap.IsActive())
    {
        overmap.SetActive(false);
    }
    else if (stash.Active)
    {
        stash.Active = false;
    }
    else if (InDialog)
    {
        advanceDialog();
    }
    else if (InMenus)
    {
        InMenus = false;
    }
    else
    {
        DisplayMenu();
    }
}

void Gui::DisplayGatherPlayers(uint16_t player_id, bool start)
{
    gathering = start;

    if (gathering)
    {
        sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
        gather_text.setString(GameManager::GetInstance().Game.GetPlayerName(player_id) + " is calling for everyone to gather at the convoy!");
        sf::FloatRect bounds = gather_text.getGlobalBounds();
        gather_text.setPosition(sf::Vector2f{window_resolution.x / 2 - bounds.width / 2, window_resolution.y * 0.15f - bounds.height / 2});
    }
}

void Gui::DisplayVote(uint16_t player_id, uint8_t vote, bool confirmed)
{
    sf::Vector2f base_vote_position;
    if (in_event)
    {
        for (unsigned i = 0; i < event_options.size(); ++i)
        {
            if (vote == i)
            {
                base_vote_position.x = event_options[i].GetGlobalBounds().left + event_options[i].GetGlobalBounds().width + 25;
                base_vote_position.y = event_options[i].GetGlobalBounds().top;
                break;
            }
        }
    }
    if (overmap.IsActive())
    {
        base_vote_position = overmap.GetBaseVoteIndicatorPosition(vote);
    }

    auto& vote_indicator = vote_indicators[player_id];

    if (confirmed)
    {
        vote_indicator.indicator.SetAnimation("Confirmed");
    }
    else
    {
        vote_indicator.indicator.SetAnimation("Unconfirmed");
    }

    float offset = 0;
    for (auto& [id, other_indicator] : vote_indicators)
    {
        if (player_id != id && other_indicator.indicator.IsVisible() && other_indicator.vote == vote)
        {
            offset += other_indicator.indicator.GetSprite().getGlobalBounds().width * 1.2f;
        }
    }

    vote_indicator.indicator.SetPosition(base_vote_position.x + offset, base_vote_position.y);
    vote_indicator.indicator.SetVisible(true);
    vote_indicator.vote = vote;

    std::set<uint16_t> checked;
    // Re-adjust other indicators
    for (auto& [id, indicator] : vote_indicators)
    {
        offset = 0;
        if (!indicator.indicator.IsVisible())
        {
            continue;
        }

        checked.insert(id);

        for (uint8_t i = 0; i < event_options.size(); ++i)
        {
            if (indicator.vote == i)
            {
                base_vote_position.x = event_options[i].GetGlobalBounds().left + event_options[i].GetGlobalBounds().width + 25;
                base_vote_position.y = event_options[i].GetGlobalBounds().top;
                break;
            }
        }

        for (auto& [other_id, other_indicator] : vote_indicators)
        {
            if (checked.contains(other_id))
            {
                continue;
            }

            if (other_indicator.indicator.IsVisible() && indicator.vote == other_indicator.vote)
            {
                offset += other_indicator.indicator.GetSprite().getGlobalBounds().width * 1.2f;
            }
        }

        indicator.indicator.SetPosition(base_vote_position.x + offset, base_vote_position.y);
    }
}

void Gui::DisplayMenu()
{
    InMenus = true;
    resume_button.SetAnimation("Up");
    save_button.SetAnimation("Up");
    settings_button.SetAnimation("Up");
    exit_button.SetAnimation("Up");
}

void Gui::DisplayDialog(std::string source, std::vector<std::string> dialog_list)
{
    dialog_text.clear();
    this->dialog = dialog_list;
    current_dialog = 0;
    setDialogText(source, dialog[0]);

    InDialog = true;
}

void Gui::DisplayMenuEvent(definitions::MenuEvent event, uint16_t page_id)
{
    current_event = event;
    in_event = true;
    event_vote_confirm_button.SetEnabled(false);
    current_vote = -1;
    event_options.clear();

    for (auto& [id, vote_indicator] : vote_indicators)
    {
        vote_indicator.vote = -1;
        vote_indicator.indicator.SetVisible(false);
    }

    sf::FloatRect background_bounds = event_background.getGlobalBounds();
    sf::FloatRect prompt_bounds = sf::FloatRect{background_bounds.left + 20, background_bounds.top + 20, background_bounds.width - 40, background_bounds.height * 0.6f};
    event_prompt.Initialize(current_event.pages[page_id].prompt, prompt_bounds);

    for (unsigned i = 0; i < current_event.pages[page_id].options.size(); ++i)
    {
        sf::Text option_text = event_prompt.DisplayText;
        option_text.setString(current_event.pages[page_id].options[i].text);
        event_options.push_back(CursorButton(option_text, sf::Color::White, sf::Color::Yellow, sf::Color::Cyan, sf::Color::Black));
        event_options[i].RegisterLeftMouseUp(std::bind(&Gui::onMenuOptionClick, this, i));
    }

    if (event_options.size() == 0)
    {
        cerr << "Menu event has no selectable options to progress." << endl;
        return;
    }

    event_options[0].GetTransform().setPosition(prompt_bounds.left, prompt_bounds.top + event_prompt.DisplayText.getGlobalBounds().height + 100);
    for (unsigned i = 1; i < event_options.size(); ++i)
    {
        event_options[i].GetTransform().setPosition(event_options[0].GetTransform().getPosition().x, event_options[0].GetTransform().getPosition().y + 50 * i);
    }
}

void Gui::AdvanceMenuEvent(uint16_t advance_value, bool finish)
{
    if (!finish)
    {
        DisplayMenuEvent(current_event, advance_value);
    }
    else
    {
        // End event actions (if any)
        in_event = false;
        GameManager::GetInstance().Game.SetPlayerActionsEnabled(true);
    }
}

void Gui::SetOvermapDisplay(bool display)
{
    for (auto& [id, vote_indicator] : vote_indicators)
    {
        vote_indicator.vote = -1;
        vote_indicator.indicator.SetVisible(false);
    }

    if (display)
    {
        overmap.SetActive(true, battery_bar.getScale().x * 1000);
        gathering = false;
    }
    else
    {
        overmap.SetActive(false);
        // TODO: There will eventually be ways to look at the overmap without interacting with the console
    }
}

void Gui::DisplayStash()
{
    stash.Active = true;
}

void Gui::exitGame()
{
    GameManager::GetInstance().Reset();
}

void Gui::setDialogText(std::string source, std::string dialog_block)
{
    dialog_source_text.setString(source);

    dialog_text.clear();
    std::string line = dialog_block;
    float text_overflow;
    float offset = 0;

    do
    {
        sf::Text text;
        text.setString(line);
        text.setFont(*font);
        text.setCharacterSize((Settings::GetInstance().WindowResolution.y * 0.3f) / 6);
        text.setPosition(sf::Vector2f{dialog_frame.getPosition().x + text.getCharacterSize() * 1.5f, dialog_frame.getPosition().y + text.getCharacterSize() + offset});
        text.setFillColor(sf::Color::Black);

        float working_area_width = dialog_frame.getGlobalBounds().width - text.getCharacterSize() * 3;
        text_overflow = text.getGlobalBounds().width - working_area_width;

        if (text_overflow > 0)
        {
            int naive_split = std::floor(working_area_width / text.getGlobalBounds().width * line.size());
            int split = line.substr(0, naive_split).find_last_of(' ');
            text.setString(line.substr(0, split));

            std::string remainder = line.substr(split, line.size());
            if (remainder[0] == ' ')
            {
                line = remainder.substr(1, remainder.size());
            }

            offset += text.getCharacterSize() * 1.1f;
        }

        dialog_text.push_back(text);
    }
    while (text_overflow > 0);
}

void Gui::advanceDialog()
{
    ++current_dialog;
    if (current_dialog == dialog.size())
    {
        InDialog = false;
    }
    else
    {
        setDialogText(dialog_source_text.getString(), dialog[current_dialog]);
    }
}

void Gui::onConfirmClick(bool confirmed)
{
    if (confirmed)
    {
        ClientMessage::CastVote(resources::GetServerSocket(), current_vote, true);
    }
}

void Gui::onMenuOptionClick(int option)
{
    current_vote = option;
    event_vote_confirm_button.SetEnabled(true);
    event_vote_confirm_button.SetToggled(false);
    network::ClientMessage::CastVote(resources::GetServerSocket(), option, false);
}

void Gui::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    if (enabled)
    {
        menu_button.UpdateMousePosition(event);
        if (InMenus)
        {
            resume_button.UpdateMousePosition(event);
            save_button.UpdateMousePosition(event);
            settings_button.UpdateMousePosition(event);
            exit_button.UpdateMousePosition(event);
        }
        else if (overmap.IsActive())
        {
            overmap.OnMouseMove(event);
        }
        else if (in_event)
        {
            for (auto& option : event_options)
            {
                option.UpdateMousePosition(event);
            }

            event_vote_confirm_button.UpdateMousePosition(event);
        }
    }
}

void Gui::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    if (enabled)
    {
        menu_button.UpdateMouseState(event, CursorButton::State::Down);
        if (InMenus)
        {
            resume_button.UpdateMouseState(event, CursorButton::State::Down);
            save_button.UpdateMouseState(event, CursorButton::State::Down);
            settings_button.UpdateMouseState(event, CursorButton::State::Down);
            exit_button.UpdateMouseState(event, CursorButton::State::Down);
        }
        else if (overmap.IsActive())
        {
            overmap.OnMouseDown(event);
        }
        else if (in_event)
        {
            for (auto& option : event_options)
            {
                option.UpdateMouseState(event, CursorButton::State::Down);
            }

            event_vote_confirm_button.UpdateMouseState(event, CursorButton::State::Down);
        }
        else if (stash.Active)
        {
            stash.OnMouseDown(event);
        }
        else if (InDialog && !InMenus && event.button == sf::Mouse::Left)
        {
            advanceDialog();
        }
    }
}

void Gui::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    if (enabled)
    {
        menu_button.UpdateMouseState(event, CursorButton::State::Up);
        if (InMenus)
        {
            resume_button.UpdateMouseState(event, CursorButton::State::Up);
            save_button.UpdateMouseState(event, CursorButton::State::Up);
            settings_button.UpdateMouseState(event, CursorButton::State::Up);
            exit_button.UpdateMouseState(event, CursorButton::State::Up);
        }
        else if (overmap.IsActive())
        {
            overmap.OnMouseUp(event);
        }
        else if (in_event)
        {
            for (auto& option : event_options)
            {
                option.UpdateMouseState(event, CursorButton::State::Up);
            }

            event_vote_confirm_button.UpdateMouseState(event, CursorButton::State::Up);
        }
    }
}

void Gui::OnTextEntered(sf::Event::TextEvent event)
{
    (void)event;
    if (enabled)
    {
    }
}

} // client
