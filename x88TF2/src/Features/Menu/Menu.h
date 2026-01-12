// Menu.h
// Unchanged, as per the provided document
#pragma once
#include "../../SDK/SDK.h"
#include <string>
#include "../src/Features/Menu/functions_menu/functions_menu.h"  // Include the new functions header

class CMenu {
public:
    bool IsOpen() const { return true; }
    bool m_bWantTextInput = false;
    bool m_bInKeybind = false;
};

namespace menu {
    void render();
    inline bool menu_locked = false;
    extern std::string tauntMessage;
    extern float tauntEndTime;
    extern text_type tauntType;

    // Statics moved from Menu.cpp (config-related)
    extern std::string config_name;
    extern bool editing_config_name[256];
    extern std::vector<std::string> config_files;
    extern int current_config_index;

    // Other statics from original
    extern int item_count;
    extern int item_countx3;
}

// Forward declaration to avoid redefinition
class CMenuEventListener;
extern CMenuEventListener g_EventListener;

MAKE_SINGLETON_SCOPED(CMenu, Menu, F);