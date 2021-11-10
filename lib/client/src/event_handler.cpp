/**************************************************************************************************
 *  File:       event_handler.cpp
 *  Class:      EventHandler
 *
 *  Purpose:    EventHandler holds hardware events from the sf::Window and allows the client to
 *              register callbacks that will fire when events are detected
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#include <iostream>
#include "event_handler.h"

using std::cout, std::endl;

namespace client {

EventHandler::EventHandler()
{
    for (int event_type = sf::Event::EventType::Closed; event_type < sf::Event::EventType::Count; ++event_type)
    {
        event_map[static_cast<sf::Event::EventType>(event_type)] = std::queue<sf::Event>{};
        callbacks[static_cast<sf::Event::EventType>(event_type)] = std::vector<FunctionCallback>{};
    }
}

EventHandler& EventHandler::GetInstance()
{
    static EventHandler handler;
    return handler;
}

uint64_t EventHandler::RegisterCallback(sf::Event::EventType event_type, std::function<void(sf::Event)> f)
{
    callbacks[event_type].push_back(FunctionCallback{callback_uid, f});
    return callback_uid++;
}

void EventHandler::UnregisterCallback(sf::Event::EventType event_type, uint64_t callback_id)
{
    auto element = std::remove_if(callbacks[event_type].begin(), callbacks[event_type].end(), [&](FunctionCallback callback){ return callback.callback_id == callback_id; });
    callbacks[event_type].erase(element);
}

void EventHandler::AddEvent(sf::Event event)
{
    event_map[event.type].push(event);
}

void EventHandler::RunCallbacks()
{
    // For every event type,
    for (auto& event_entry : event_map)
    {
        // For every event of that type saved in the callback queue,
        while (!event_entry.second.empty())
        {
            sf::Event event = event_entry.second.front();
            event_entry.second.pop();

            // For every callback function for the event,
            for (auto& callback : callbacks[event.type])
            {
                // Call the callback
                callback.f(event);
            }
        }
    }
}

} // client
