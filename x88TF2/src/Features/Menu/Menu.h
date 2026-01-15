// Menu.h
#pragma once
#include "../../SDK/SDK.h"
#include <string>
#include <vector>

// Define text_type here to ensure it's available (in case of include issues)
enum text_type {
    info = 0,
    regular,
    enabled,
    enabled_green,
    warning,
    extra
};

// forward-declare menu::is_open so CMenu::IsOpen() can use it
namespace menu { extern bool is_open; }

class CMenu {
public:
    bool IsOpen() const { return menu::is_open; }
    bool m_bWantTextInput = false;
    bool m_bInKeybind = false;
};

namespace menu {
    void render();
    inline bool menu_locked = false;
    extern bool is_open;  // Added for menu toggle
    extern std::string tauntMessage;
    extern float tauntEndTime;
    extern text_type tauntType;

    // Config-related statics
    extern std::string config_name;
    extern bool editing_config_name[256];
    extern std::vector<std::string> config_files;
    extern int current_config_index;

    // Other statics
    extern int item_count;
    extern int item_countx3;

    // ============= TAB E DRAG SYSTEM =============
    enum class Tab {
        AIMBOT = 0,
        ANTIAIM = 1,
        VISUALS = 2,
        SKINS = 3,
        MISC = 4
    };
    extern Tab current_tab;

    // Drag variables
    extern int menu_x;
    extern int menu_y;
    extern bool is_dragging;
    extern int drag_offset_x;
    extern int drag_offset_y;

    // ============= SCROLL SYSTEM =============
    // Variáveis de scroll por tab
    extern int scroll_offset_aimbot;
    extern int scroll_offset_antiaim;
    extern int scroll_offset_visuals;
    extern int scroll_offset_skins;
    extern int scroll_offset_misc;

    // Área visível de conteúdo
    extern int content_visible_height;
    extern int total_content_height;

    // ScrollBar
    extern bool scrollbar_dragging;
    extern int scrollbar_drag_start_y;
    extern int scrollbar_drag_start_offset;

    // ============= HELPER FUNCTIONS =============
    bool IsMouseInRect(int x, int y, int w, int h);
    void HandleDrag();
    void HandleTabClick();
    void HandleScroll();

    // Scroll system functions
    int& GetCurrentScrollOffset();
    void ResetScrollOnTabChange();

    // Deferred render helpers (notifications / popups)
    void BeginDeferredRender();                      // limpar fila antes do frame
    void RenderDeferredPopups();                     // desenhar popups após desativar clipping
    void PushDeferredPopup(std::function<void()> f); // empurra função de desenho para a fila
}

// Forward declaration
class CMenuEventListener;
extern CMenuEventListener g_EventListener;

MAKE_SINGLETON_SCOPED(CMenu, Menu, F);