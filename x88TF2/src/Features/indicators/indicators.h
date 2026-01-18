#pragma once
#include "../src/imgui/imgui.h"

// Forward declaration do namespace gui
namespace gui {
    extern ImFont* indicator_font;
}

namespace indicators {
    extern bool local_player_death;
    extern bool player_died[64];

    void indicator();
    void keybind();
    void watermark();
	void spectator_list();
    void Run();
}