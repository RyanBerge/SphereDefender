/**************************************************************************************************
 *  File:       event_handler.h
 *  Class:      EventHandler
 *
 *  Purpose:    EventHandler holds hardware events from the sf::Window and allows the client to
 *              register callbacks that will fire when events are detected
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include <SFML/Window/Event.hpp>
#include <functional>
#include <map>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>

namespace client {

class EventHandler
{
public:
    struct FunctionCallback {
        uint64_t callback_id;
        std::function<void(sf::Event)> f;
        std::shared_ptr<bool> valid;
    };

    EventHandler(const EventHandler&) = delete;
    EventHandler(EventHandler&&) = delete;

    EventHandler& operator=(const EventHandler&) = delete;
    EventHandler& operator=(EventHandler&&) = delete;

    static EventHandler& GetInstance();

    uint64_t RegisterCallback(sf::Event::EventType event_type, std::function<void(sf::Event)> f);
    void UnregisterCallback(sf::Event::EventType event_type, uint64_t callback_id);
    void AddEvent(sf::Event event);
    void RunCallbacks();

private:
    EventHandler();

    uint64_t callback_uid = 0;
    std::map<sf::Event::EventType, std::queue<sf::Event>> event_map;
    std::recursive_mutex event_mutex;

    // TODO: Priorities for callbacks?
    std::map<sf::Event::EventType, std::vector<FunctionCallback>> callbacks;
};

} // client
