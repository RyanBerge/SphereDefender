/**************************************************************************************************
 *  File:       toggle_button.h
 *  Class:      ToggleButton
 *
 *  Purpose:    ToggleButton represents an input button that can be clicked with the mouse
 *
 *  Author:     Ryan Berge
 *
 *************************************************************************************************/
#pragma once

#include "cursor_button.h"

namespace client {

class ToggleButton : public CursorButton
{
public:
    ToggleButton();
    ToggleButton(std::string filepath);

    virtual void Toggle();
    virtual void SetToggled(bool toggle_value);
    virtual bool GetToggled();
    virtual void RegisterOnToggle(std::function<void(bool)> f);

protected:
    virtual void onLeftMouseUp(bool in_bounds) override;

    bool toggled = false;

    std::vector<std::function<void(bool)>> toggleCallbacks;
};

} // client
