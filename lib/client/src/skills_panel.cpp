/**************************************************************************************************
 *  File:       shop_window.h
 *  Class:      ShopWindow
 *
 *  Purpose:    Represents a shop window
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <iostream>
#include "skills_panel.h"
#include "resources.h"
#include "settings.h"
#include "messaging.h"
#include "player_stats.h"
#include "settings.h"

using std::cout, std::endl;
using network::ClientMessage;

namespace client {

SkillsPanel::SkillsPanel()
{
    IsActive = false;
    player_class = definitions::GetPlayerClass(definitions::PlayerClassType::Melee); // TODO: Get class?

    sf::Vector2f window_resolution = Settings::GetInstance().WindowResolution;
    font = resources::FontManager::GetFont("Vera");

    panel.LoadAnimationData("gui/skills_panel.json");
    sf::FloatRect panel_bounds = panel.GetSprite().getGlobalBounds();
    panel.SetPosition(sf::Vector2f{window_resolution.x / 2 - panel_bounds.width / 2, window_resolution.y / 2 - panel_bounds.height / 2});
    panel_bounds = panel.GetSprite().getGlobalBounds();
    sf::Vector2f panel_center{panel_bounds.left + panel_bounds.width / 2, panel_bounds.top + panel_bounds.height / 2};

    background.setFillColor(sf::Color{0, 0, 0, 128});
    background.setPosition(sf::Vector2f{0, 0});
    background.setSize(window_resolution);

    x_button.LoadAnimationData("gui/x_button.json");
    sf::FloatRect x_bounds = x_button.GetSprite().getGlobalBounds();
    x_button.SetPosition(panel_bounds.left + panel_bounds.width - x_bounds.width - 3, panel_bounds.top + 3);
    x_button.RegisterLeftMouseUp([this](void){ IsActive = false; });

    for (auto& skill_row : player_class.skills)
    {
        skills.push_back(std::vector<SkillButton>());
        for (auto& skill : skill_row)
        {
            SkillButton skill_button;
            //skill_button.button.SetMode(ToggleButton::Mode::Animation);
            skill_button.button.LoadAnimationData("gui/skill_frame.json");
            sf::Vector2f button_position;
            button_position.x = panel_center.x - skill_button.button.GetGlobalBounds().width / 2;
            button_position.y = panel_center.y + 70 - skill_button.button.GetGlobalBounds().height * 1.5 * skill.row;
            skill_button.button.SetPosition(button_position);

            skill_button.text.setFont(*font);
            skill_button.text.setString(skill.name);
            skill_button.text.setCharacterSize(12);
            skill_button.text.setOrigin(sf::Vector2f{skill_button.text.getGlobalBounds().width / 2, skill_button.text.getGlobalBounds().height / 2});
            sf::FloatRect skill_button_bounds = skill_button.button.GetGlobalBounds();
            skill_button.text.setPosition(sf::Vector2f{skill_button_bounds.left + skill_button_bounds.width / 2, skill_button_bounds.top + skill_button_bounds.height / 2});
            skill_button.text.setFillColor(sf::Color::Black);
            skill_button.button.SetMode(ToggleButton::Mode::Tint);
            //skill_button.text.setOutlineColor(sf::Color::White);
            //skill_button.text.setOutlineThickness(1);
            skills[skill.row].push_back(skill_button);

            skill_description.setFont(*font);
            skill_description.setCharacterSize(14);
            skill_description.setPosition(panel_bounds.left + 15, panel_center.y + 150);
            skill_description.setFillColor(sf::Color::Black);
            skill_description.setString("");

            skill_points.setFont(*font);
            skill_points.setCharacterSize(35);
            skill_points.setPosition(panel_bounds.left + panel_bounds.width - 30, panel_bounds.top + panel_bounds.height - 50);
            skill_points.setFillColor(sf::Color::Black);
            skill_points.setString("0");

            skills[skill.row][skills[skill.row].size() - 1].button.RegisterCursorEnter([this, skill](void){
                skill_description.setString(skill.description);
            });

            skills[skill.row][skills[skill.row].size() - 1].button.RegisterCursorExit([this, skill](void){
                skill_description.setString("");
            });

            skills[skill.row][skills[skill.row].size() - 1].button.RegisterLeftMouseUp([this, skill](void){
                if (stats::GetPlayerStats().UnspentSkillPoints > 0)
                {
                    stats::GetPlayerStats().UnspentSkillPoints -= 1;
                    stats::GetPlayerStats().Skills[skill.type] = true;
                    Settings::GetInstance().BindSkill(skill);
                }
            });
        }
    }
}

void SkillsPanel::Draw()
{
    if (IsActive)
    {
        resources::GetWindow().draw(background);
        panel.Draw();
        x_button.Draw();

        for (auto& row : skills)
        {
            for (auto& skill_button : row)
            {
                skill_button.button.Draw();
                resources::GetWindow().draw(skill_button.text);
                resources::GetWindow().draw(skill_description);
                skill_points.setString(std::to_string(stats::GetPlayerStats().UnspentSkillPoints));
                resources::GetWindow().draw(skill_points);
            }
        }
    }
}

void SkillsPanel::OnMouseDown(sf::Event::MouseButtonEvent event)
{
    if (IsActive && stats::GetPlayerStats().UnspentSkillPoints > 0)
    {
        x_button.UpdateMouseState(event, CursorButton::State::Down);
        for (auto& row : skills)
        {
            for (auto& skill : row)
            {
                skill.button.UpdateMouseState(event, CursorButton::State::Down);
            }
        }
    }
}

void SkillsPanel::OnMouseUp(sf::Event::MouseButtonEvent event)
{
    if (IsActive && stats::GetPlayerStats().UnspentSkillPoints > 0)
    {
        x_button.UpdateMouseState(event, CursorButton::State::Up);
        for (auto& row : skills)
        {
            for (auto& skill : row)
            {
                skill.button.UpdateMouseState(event, CursorButton::State::Up);
            }
        }
    }
}

void SkillsPanel::OnMouseMove(sf::Event::MouseMoveEvent event)
{
    if (IsActive)
    {
        x_button.UpdateMousePosition(event);
        for (auto& row : skills)
        {
            for (auto& skill : row)
            {
                skill.button.UpdateMousePosition(event);
            }
        }
    }
}

} // namespace client
