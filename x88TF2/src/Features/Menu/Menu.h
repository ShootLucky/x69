// Menu.h
#pragma once
#include "../../SDK/SDK.h"
#include <string>

enum text_type {
    info = 0,
    regular,
    enabled,
    enabled_green,
    warning,
    extra
};

namespace menu {
    void render();
    inline bool menu_locked = false;
    extern std::string tauntMessage;
    extern float tauntEndTime;
    extern text_type tauntType;
}

// Forward declaration to avoid redefinition
class CMenuEventListener;
extern CMenuEventListener g_EventListener;