#pragma once
#include "igameevents.h"

class CGameEventListener : public IGameEventListener2
{
public:
    virtual void FireGameEvent(IGameEvent* event) = 0;
    virtual int GetEventDebugID(void) const { return 0; }  // Added for SDK compatibility
};