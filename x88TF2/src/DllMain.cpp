#include "includes.h"
#include "App/App.h"
#include "../src/Features/SkinChanger/SkinChanger.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <Psapi.h>
#define STB_IMAGE_IMPLEMENTATION
#include "icons.h"
#include "stb_image.h"
#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif
#include "../nemesis.h"

namespace PatternScan
{
    std::vector<int> PatternToBytes(const char* pattern)
    {
        std::vector<int> bytes;
        char* start = const_cast<char*>(pattern);
        char* end = const_cast<char*>(pattern) + strlen(pattern);

        for (char* current = start; current < end; ++current)
        {
            if (*current == '?')
            {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else
            {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    }

    uintptr_t FindPattern(const char* moduleName, const char* pattern)
    {
        HMODULE module = GetModuleHandleA(moduleName);
        if (!module)
            return 0;

        MODULEINFO moduleInfo;
        if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(MODULEINFO)))
            return 0;

        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(module);
        uintptr_t moduleEnd = moduleBase + moduleInfo.SizeOfImage;

        std::vector<int> patternBytes = PatternToBytes(pattern);

        for (uintptr_t i = moduleBase; i < moduleEnd - patternBytes.size(); ++i)
        {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); ++j)
            {
                if (patternBytes[j] != -1 && patternBytes[j] != *(reinterpret_cast<uint8_t*>(i + j)))
                {
                    found = false;
                    break;
                }
            }

            if (found)
                return i;
        }

        return 0;
    }

    uintptr_t FindPatternWithOffset(const char* moduleName, const char* pattern, int offset = 0, int relativeOffset = 0)
    {
        uintptr_t address = FindPattern(moduleName, pattern);
        if (!address)
            return 0;

        address += offset;

        if (relativeOffset != 0)
        {
            int32_t rel = *reinterpret_cast<int32_t*>(address);
            address = address + relativeOffset + rel;
        }

        return address;
    }
}

// ============================================================================
// PATTERN SCANNING INITIALIZATION
// ============================================================================
bool InitializePatternScanning()
{
    // Check if client.dll is loaded
    HMODULE clientDll = GetModuleHandleA("client.dll");
    if (!clientDll)
    {
        MessageBoxA(nullptr,
            "Failed to find client.dll!\n\n"
            "Make sure the game is running.",
            "Pattern Scan Error",
            MB_OK | MB_ICONERROR);
        return false;
    }

    // Pattern scanning will be done by the existing Signatures system
    // We just need to verify the signatures are initialized

    // The signatures are already defined in SkinChanger.cpp:
    // - GetItemSchema
    // - CEconItemSchema_GetAttributeDefinition  
    // - CAttributeList_SetRuntimeAttributeValue

    // These signatures use the SIGNATURE() and MAKE_SIGNATURE() macros
    // which automatically scan when .Get() is called

    // We can enable pattern scanning mode in SkinChanger
    g_SkinChanger.SetPatternScanningMode(true);

    return true;
}


static IDirect3DTexture9* icon_texture = nullptr;
static int selected_weapon = 0;
static std::vector<bool> hitbox_selected = { false, false, false, false, false };
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

bool init = false;
HMODULE hmod = NULL;

IDirect3DStateBlock9* pStateBlock = NULL;

// Config system variables
static std::vector<std::string> config_list;
static int selected_config = 0;
static char new_config_name[128] = "";

static int sc_selected_weapon_slot = 0;
static int sc_paintkit_id = 0;
static int sc_seed = 0;
static float sc_wear = 0.01f;
static int sc_effect_id = 0;
static bool sc_australium = false;
static bool sc_festivized = false;
static int sc_killstreak_tier = 0;
static int sc_killstreak_effect = 0;
static char sc_attribute_input[32] = "";
static char sc_value_input[32] = "";


IDirect3DTexture9* LoadTextureFromMemory(LPDIRECT3DDEVICE9 device, const unsigned char* data, int data_size)
{
    int width, height, channels;
    unsigned char* image_data = stbi_load_from_memory(data, data_size, &width, &height, &channels, 4);

    if (!image_data)
        return nullptr;

    IDirect3DTexture9* texture = nullptr;
    HRESULT hr = device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr);

    if (FAILED(hr))
    {
        stbi_image_free(image_data);
        return nullptr;
    }

    D3DLOCKED_RECT rect;
    if (SUCCEEDED(texture->LockRect(0, &rect, nullptr, 0)))
    {
        for (int y = 0; y < height; y++)
        {
            memcpy(
                (unsigned char*)rect.pBits + rect.Pitch * y,
                image_data + (width * 4) * y,
                width * 4
            );
        }
        texture->UnlockRect(0);
    }

    stbi_image_free(image_data);
    return texture;
}

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX9_Init(pDevice);

    gui::set_theme();
    gui::menu_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\verdanab.ttf", 13.f);
    gui::indicator_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Tahoma.ttf", 12.f);

    // CARREGA OS ÍCONES AQUI
    icon_texture = LoadTextureFromMemory(pDevice, qo0_icons, sizeof(qo0_icons));

    // Initialize config list
    config_list = Config::RefreshConfigFiles();

    // Load default config if it exists
    std::string default_path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Team Fortress 2\\phantom\\Configs\\default.cfg";
    if (std::filesystem::exists(default_path))
    {
        Config::LoadConfig("default");
        for (size_t i = 0; i < config_list.size(); i++)
        {
            if (config_list[i] == "default")
            {
                selected_config = i;
                break;
            }
        }
    }
}

IDirect3DStateBlock9* pixel_state = NULL;
IDirect3DVertexDeclaration9* vertDec;
IDirect3DVertexShader9* vertShader;
DWORD dwOld_D3DRS_COLORWRITEENABLE;

void SaveState(IDirect3DDevice9* pDevice)
{
    pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
    pDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &pixel_state);
    pDevice->GetVertexDeclaration(&vertDec);
    pDevice->GetVertexShader(&vertShader);
    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
    pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
    pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
    pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
}

void RestoreState(IDirect3DDevice9* pDevice)
{
    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
    pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
    pixel_state->Apply();
    pixel_state->Release();
    pDevice->SetVertexDeclaration(vertDec);
    pDevice->SetVertexShader(vertShader);
}

bool show_menu = true;

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    if (!alive)
        return oEndScene(pDevice);

    SaveState(pDevice);

    if (!init)
    {
        InitImGui(pDevice);
        init = true;
    }

    if (pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock) == D3D_OK)
    {
        pStateBlock->Capture();

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        indicators::Run();

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            show_menu = !show_menu;
        }
        if (GetAsyncKeyState(VK_END) & 0x8000)
        {
            alive = false;
        }
        if (show_menu)
        {
            static int active_tab = 0;
            ImGui::SetNextWindowSize(ImVec2(800, 600));
            ImGui::Begin("SLwindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetWindowPos();
                ImVec2 s = ImGui::GetWindowSize();
                draw_list->AddRectFilledMultiColor
                (
                    p,
                    ImVec2(p.x + s.x, p.y + 0),
                    IM_COL32(24, 24, 24, 255),
                    IM_COL32(24, 24, 24, 255),
                    IM_COL32(16, 16, 16, 255),
                    IM_COL32(16, 16, 16, 255)
                );
                // CORREÇÃO AQUI: Usar Color_t em vez de gui::Color
                static bool modify_theme = CFG::Menu_ModifyTheme;
                static Color_t theme_color = CFG::Menu_ThemeColor;
                static bool old_modify_theme = false;
                static Color_t old_theme_color = theme_color;
                ImU32 accent_color = modify_theme ? IM_COL32(theme_color.r, theme_color.g, theme_color.b, theme_color.a) : IM_COL32(0, 122, 187, 255);

                // Linha colorida no topo
                draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + 1), ImVec2(p.x + s.x - 1, p.y + 3), accent_color);

                // Linha cinza fina embaixo do header
                draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + 59), ImVec2(p.x + s.x - 1, p.y + 60), IM_COL32(64, 64, 64, 255));

                if (icon_texture)
                {
                    // Desenha o ícone no lugar do P (tamanho pequeno para caber no header)
                    ImVec2 icon_pos(p.x + -4, p.y + -2);
                    ImVec2 icon_size(78, 68); // Ícone 48x48 pixels
                    // Calcular a cor do tema para aplicar no ícone
                    ImVec4 tint_color = modify_theme
                        ? ImVec4(theme_color.r / 255.0f, theme_color.g / 255.0f, theme_color.b / 255.0f, 1.0f)
                        : ImVec4(0.0f, 122.0f / 255.0f, 187.0f / 255.0f, 1.0f);
                    // Desenhar ícone com a cor do tema aplicada
                    draw_list->AddImage(
                        (void*)icon_texture,
                        icon_pos,
                        ImVec2(icon_pos.x + icon_size.x, icon_pos.y + icon_size.y),
                        ImVec2(0, 0),  // uv0
                        ImVec2(1, 1),  // uv1
                        ImGui::ColorConvertFloat4ToU32(tint_color)  // Aplica a cor do tema
                    );

                    float icon_end = p.x + 60; // Posição ajustada após o ícone

                    // Push font scale para aumentar o tamanho do texto em 1.5x
                    ImGui::PushFont(ImGui::GetFont());
                    ImGui::SetWindowFontScale(1.5f);

                    // Sombra do texto HANTOM.CLUB
                    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                        ImVec2(icon_end + 1, p.y + 16), IM_COL32(5, 5, 5, 255), "HANTOM.CLUB");
                    // Texto principal HANTOM.CLUB
                    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                        ImVec2(icon_end, p.y + 15), accent_color, "HANTOM.CLUB");

                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::PopFont();
                }
                else
                {
                    // Push font scale para aumentar o tamanho do texto em 1.5x
                    ImGui::PushFont(ImGui::GetFont());
                    ImGui::SetWindowFontScale(1.5f);

                    // Sombra do texto PHANTOM.CLUB
                    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                        ImVec2(p.x + -24, p.y + 21), IM_COL32(5, 5, 5, 255), "PHANTOM.CLUB");
                    // Texto principal PHANTOM.CLUB
                    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                        ImVec2(p.x + -25, p.y + 20), accent_color, "PHANTOM.CLUB");

                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::PopFont();
                }

                draw_list->AddText(ImVec2(p.x + 60, p.y + 33), IM_COL32(5, 5, 5, 255), "DEVELOPED BY");
                draw_list->AddText(ImVec2(p.x + 61, p.y + 32), IM_COL32(255, 255, 255, 100), "DEVELOPED BY");
                float dev_width = ImGui::CalcTextSize("DEVELOPED BY").x;
                draw_list->AddText(ImVec2(p.x + 60 + dev_width + 5, p.y + 33), IM_COL32(5, 5, 5, 255), "SHOOT & VOID");
                draw_list->AddText(ImVec2(p.x + 61 + dev_width + 4, p.y + 32), accent_color, "SHOOT & VOID");
                int grid_size = 6; // pixels por célula
                int header_height = 60;
                int cols = std::max(1, (int)(s.x / grid_size));
                int rows = std::max(1, (int)((s.y - header_height) / grid_size));
                draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + header_height), ImVec2(p.x + s.x - 1, p.y + s.y - 1), IM_COL32(16, 16, 16, 255));
                ImU32 lineColor = IM_COL32(26, 26, 26, 200);
                for (int i = 0; i <= cols; ++i) {
                    float x = p.x + i * grid_size + 0.5f;
                    draw_list->AddLine(ImVec2(x, p.y + header_height), ImVec2(x, p.y + s.y), lineColor, 1.0f);
                }
                for (int j = 0; j <= rows; ++j) {
                    float y = p.y + header_height + j * grid_size + 0.5f;
                    draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + s.x, y), lineColor, 1.0f);
                }
                struct Symbol {
                    float x, y;      // posição em células (float)
                    float vx, vy;    // direção normalizada
                    float speedMul;  // multiplicador de velocidade
                    char ch;         // caractere fixo
                    float alpha;     // transparência
                    float sizeMul;   // tamanho
                };
                static std::vector<Symbol> symbols;
                static bool symbols_init = false;
                static int last_cols = 0;
                static int last_rows = 0;
                const int MAX_SYMBOLS_CAP = 80; // teto para manter desempenho
                int area = cols * rows;
                int computedMax = std::clamp(area / 24, 6, MAX_SYMBOLS_CAP);
                if (!symbols_init || last_cols != cols || last_rows != rows) {
                    symbols_init = true;
                    last_cols = cols;
                    last_rows = rows;
                    symbols.clear();
                    symbols.reserve(computedMax);
                    srand((unsigned)time(NULL));

                    const char symbolSet[] = "$#&%*+-=~<>/\\|@?";
                    for (int i = 0; i < computedMax; ++i) {
                        Symbol sbol;
                        sbol.x = (float)(rand() % cols);
                        sbol.y = (float)(rand() % rows);
                        float ang = (float)((rand() / (float)RAND_MAX) * (2.0f * 3.14159265f));
                        sbol.vx = cosf(ang);
                        sbol.vy = sinf(ang);
                        sbol.speedMul = 0.6f + (rand() % 80) / 100.0f;   // 0.6 .. 1.39
                        sbol.ch = symbolSet[rand() % (sizeof(symbolSet) - 1)];
                        sbol.alpha = 0.55f + (rand() % 40) / 100.0f;     // 0.55 .. 0.94
                        sbol.sizeMul = 0.85f + (rand() % 35) / 100.0f;   // 0.85 .. 1.19
                        symbols.push_back(sbol);
                    }
                }
                ImGuiIO& io_local = ImGui::GetIO();
                float dt = io_local.DeltaTime;
                const float baseSpeed = 5.5f; // células por segundo (mais rápido)
                const float directionChangeRatePerSec = 0.45f; // chance por segundo de alterar direção
                const float minSeparation = 0.55f; // distância mínima (em células) para evitar sobreposição
                const float separationStrength = 0.6f; // quanto empurrar ao colidir
                const float velocityDamping = 0.08f; // reduz velocidade ao colidir para estabilidade
                for (auto& sym : symbols) {
                    // pequena variação suave na direção
                    if ((rand() / (float)RAND_MAX) < (directionChangeRatePerSec * dt)) {
                        float deltaAng = ((rand() / (float)RAND_MAX) - 0.5f) * (3.14159265f / 4.0f); // ±45°
                        float ang = atan2f(sym.vy, sym.vx) + deltaAng;
                        sym.vx = cosf(ang);
                        sym.vy = sinf(ang);
                    }
                    sym.x += sym.vx * sym.speedMul * baseSpeed * dt;
                    sym.y += sym.vy * sym.speedMul * baseSpeed * dt;
                    if (sym.x < 0.0f) sym.x += cols;
                    if (sym.x >= (float)cols) sym.x -= cols;
                    if (sym.y < 0.0f) sym.y += rows;
                    if (sym.y >= (float)rows) sym.y -= rows;
                }
                for (size_t i = 0; i < symbols.size(); ++i) {
                    for (size_t j = i + 1; j < symbols.size(); ++j) {
                        Symbol& a = symbols[i];
                        Symbol& b = symbols[j];
                        float dx = b.x - a.x;
                        float dy = b.y - a.y;
                        if (dx > cols * 0.5f) dx -= cols;
                        if (dx < -cols * 0.5f) dx += cols;
                        if (dy > rows * 0.5f) dy -= rows;
                        if (dy < -rows * 0.5f) dy += rows;
                        float distSq = dx * dx + dy * dy;
                        float minDist = minSeparation;
                        if (distSq < (minDist * minDist) && distSq > 0.0001f) {
                            float dist = sqrtf(distSq);
                            float overlap = (minDist - dist) * separationStrength;
                            float nx = dx / dist;
                            float ny = dy / dist;
                            a.x -= nx * (overlap * 0.5f);
                            a.y -= ny * (overlap * 0.5f);
                            b.x += nx * (overlap * 0.5f);
                            b.y += ny * (overlap * 0.5f);
                            a.vx -= nx * velocityDamping;
                            a.vy -= ny * velocityDamping;
                            b.vx += nx * velocityDamping;
                            b.vy += ny * velocityDamping;
                            float la = sqrtf(a.vx * a.vx + a.vy * a.vy);
                            if (la > 0.0001f) { a.vx /= la; a.vy /= la; }
                            float lb = sqrtf(b.vx * b.vx + b.vy * b.vy);
                            if (lb > 0.0001f) { b.vx /= lb; b.vy /= lb; }
                        }
                        else if (distSq <= 0.0001f) {
                            float ang = (rand() / (float)RAND_MAX) * 2.0f * 3.14159265f;
                            float nx = cosf(ang), ny = sinf(ang);
                            a.x -= nx * 0.2f; a.y -= ny * 0.2f;
                            b.x += nx * 0.2f; b.y += ny * 0.2f;
                        }
                    }
                }
                for (auto& sym : symbols) {
                    while (sym.x < 0.0f) sym.x += cols;
                    while (sym.x >= (float)cols) sym.x -= cols;
                    while (sym.y < 0.0f) sym.y += rows;
                    while (sym.y >= (float)rows) sym.y -= rows;
                }
                ImVec4 themeFloat;
                themeFloat.x = ((modify_theme ? theme_color.r : 0) / 255.0f);
                themeFloat.y = ((modify_theme ? theme_color.g : 122) / 255.0f);
                themeFloat.z = ((modify_theme ? theme_color.b : 187) / 255.0f);
                themeFloat.w = 1.0f;
                ImFont* font = ImGui::GetFont();
                float baseFontSize = ImGui::GetFontSize();
                for (const auto& sym : symbols) {
                    float cx = p.x + (sym.x * grid_size) + grid_size * 0.5f;
                    float cy = p.y + header_height + (sym.y * grid_size) + grid_size * 0.5f;
                    ImU32 symCol = ImGui::ColorConvertFloat4ToU32(ImVec4(themeFloat.x, themeFloat.y, themeFloat.z, sym.alpha));
                    float fontSize = baseFontSize * sym.sizeMul;
                    char buf[4] = { sym.ch, '\0', '\0', '\0' };
                    draw_list->AddText(font, fontSize, ImVec2(cx - fontSize * 0.35f, cy - fontSize * 0.7f), symCol, buf);
                }
                draw_list->AddRect(p, ImVec2(p.x + s.x, p.y + s.y), IM_COL32(40, 40, 40, 255));

                ImGui::SetCursorPosX(430);
                ImGui::SetCursorPosY(25);
                ImGui::BeginGroup();
                {
                    if (gui::TabButton("Aimbot", active_tab, 0)) {
                        // Clique esquerdo normal
                    }
                    // Detectar clique direito no Aimbot
                    ImVec2 aimbot_btn_min = ImGui::GetItemRectMin();
                    ImVec2 aimbot_btn_max = ImGui::GetItemRectMax();
                    if (ImGui::IsMouseHoveringRect(aimbot_btn_min, aimbot_btn_max) &&
                        ImGui::IsMouseClicked(1)) {
                        gui::g_aimbot_mode_state.context_open = true;
                    }
                    gui::aimbot_mode_selector(&gui::g_aimbot_mode_state.selected_mode);
                    ImGui::SameLine();
                    gui::TabButton("Anti-Aim", active_tab, 1);
                    ImGui::SameLine();
                    if (gui::TabButton("Visuals", active_tab, 2)) {
                        // Clique esquerdo normal
                    }
                    // Detectar clique direito
                    ImVec2 visuals_btn_min = ImGui::GetItemRectMin();
                    ImVec2 visuals_btn_max = ImGui::GetItemRectMax();
                    if (ImGui::IsMouseHoveringRect(visuals_btn_min, visuals_btn_max) &&
                        ImGui::IsMouseClicked(1)) {
                        gui::g_visuals_mode_state.context_open = true;
                    }
                    gui::visuals_mode_selector(&gui::g_visuals_mode_state.selected_mode);
                    ImGui::SameLine();
                    gui::TabButton("Players", active_tab, 3);
                    ImGui::SameLine();
                    gui::TabButton("Skin Changer", active_tab, 4);  // <-- NOVA TAB
                    ImGui::SameLine();
                    gui::TabButton("Misc", active_tab, 5);          // <-- MUDE DE 4 PARA 5
                }
                ImGui::EndGroup();
                ImGui::NewLine();
                ImGui::NewLine();

                /* main content */
                ImGui::SetCursorPosX(15);
                ImGui::BeginGroup();
                {
                    // AIMBOT TAB
                    if (active_tab == 0)
                    {
                        int aimbot_mode = gui::g_aimbot_mode_state.selected_mode;

                        // ===== AIMBOT MODE (0) =====
                        if (aimbot_mode == 0) {
                            if (gui::begin_group_scrollable("MAIN", ImVec2(380, 250), 5.0f, 0.0f))
                            {
                                ImGui::BeginGroup();
                                gui::checkbox("Aimbot", CFG::Aimbot_Active);
                                ImGui::SameLine(350.0f);
                                static int aimbot_key = CFG::Aimbot_Key;
                                static int aimbot_bind_type = CFG::Aimbot_KeyMode;
                                if (aimbot_key != CFG::Aimbot_Key || aimbot_bind_type != CFG::Aimbot_KeyMode)
                                {
                                    aimbot_key = CFG::Aimbot_Key;
                                    aimbot_bind_type = CFG::Aimbot_KeyMode;
                                }
                                gui::keybind("##AimbotKey", &aimbot_key, &aimbot_bind_type);
                                CFG::Aimbot_Key = aimbot_key;
                                CFG::Aimbot_KeyMode = aimbot_bind_type;
                                ImGui::EndGroup();
                                gui::slider("FOV", &CFG::Aimbot_FOV, 0.f, 180.f);
                                gui::combo("Aim Type", &CFG::Aimbot_Hitscan_Mode,
                                    std::vector<std::string>{ "Plain", "Silent", "Smooth" });
                                if (CFG::Aimbot_Hitscan_Mode == 2) // Smooth mode
                                {
                                    gui::slider("Smoothing", &CFG::Aimbot_Hitscan_Smoothing, 0.f, 100.f);
                                }
                                gui::checkbox("Auto Shoot", CFG::Aimbot_AutoShoot);
                                gui::checkbox("Active Lag Records", CFG::Aimbot_ActiveLagRecords);
                                gui::combo("Sort", &CFG::Aimbot_Hitscan_Sort,
                                    std::vector<std::string>{ "FOV", "Distance", "Health" });
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);
                            if (gui::begin_group("ACCURACY", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                gui::checkbox("Wait For Headshot", CFG::Aimbot_WaitForHeadshot);
                                gui::checkbox("Wait For Charge", CFG::Aimbot_WaitForCharge);
                                gui::checkbox("Minigun Tapfire", CFG::Aimbot_MinigunTapfire);
                                gui::checkbox("Auto Scope", CFG::Aimbot_AutoScope);
                            }
                            gui::end_group();

                            if (gui::begin_group_scrollable("TARGET", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                gui::checkbox("Target Lag Records", CFG::Aimbot_TargetLagRecords);
                                gui::checkbox("Target Stickies", CFG::Aimbot_TargetStickies);
                                gui::checkbox("Target Players", CFG::Aimbot_Target_Players);
                                gui::checkbox("Target Buildings", CFG::Aimbot_Target_Buildings);

                                std::vector<gui::MultiComboItem> items = {
                                    gui::MultiComboItem("Head", &CFG::Aimbot_Hitbox_Head),
                                    gui::MultiComboItem("Body", &CFG::Aimbot_Hitbox_Body),
                                    gui::MultiComboItem("Pelvis", &CFG::Aimbot_Hitbox_Pelvis),
                                    gui::MultiComboItem("Arms", &CFG::Aimbot_Hitbox_Arms),
                                    gui::MultiComboItem("Legs", &CFG::Aimbot_Hitbox_Legs)
                                };
                                gui::multi_combo("Hitbox Types", items);

                                gui::combo("Hitbox Sort", &CFG::Aimbot_Hitbox_Sort, std::vector<std::string>{ "Auto", "Damage", "Accuracy" });
                                gui::checkbox("Ignore Invisible", CFG::Aimbot_Ignore_Invisible);
                                gui::checkbox("Ignore Taunting", CFG::Aimbot_Ignore_Taunting);
                                gui::checkbox("Ignore Invulnerable", CFG::Aimbot_Ignore_Invulnerable);
                                gui::checkbox("Visible Check", CFG::Aimbot_VisibleCheck);
                                gui::checkbox("Team Check", CFG::Aimbot_TeamCheck);
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);
                            if (gui::begin_group("EXPLOITS", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                ImGui::BeginGroup();
                                gui::checkbox("Shifting", CFG::shifting_active);
                                ImGui::SameLine(350.0f);
                                static int shifting_key = CFG::shifting_key;
                                static int shifting_bind_type = CFG::shifting_key_mode;
                                if (shifting_key != CFG::shifting_key || shifting_bind_type != CFG::shifting_key_mode)
                                {
                                    shifting_key = CFG::shifting_key;
                                    shifting_bind_type = CFG::shifting_key_mode;
                                }
                                gui::keybind("##ShiftingKey", &shifting_key, &shifting_bind_type);
                                CFG::shifting_key = shifting_key;
                                CFG::shifting_key_mode = shifting_bind_type;
                                ImGui::EndGroup();
                                gui::slider("Delay Ticks", &CFG::shifting_delay_ticks, 0.f, 20.f);
                                gui::slider("Delay Hitscan", &CFG::shifting_delay_hitscan, 0.f, 10.f);
                                ImGui::BeginGroup();
                                gui::checkbox("Shifting Recharge", CFG::shifting_active);
                                ImGui::SameLine(350.0f);
                                static int recharge_key = CFG::shifting_recharge_key;
                                static int recharge_bind_type = CFG::shifting_recharge_key_mode;
                                if (recharge_key != CFG::shifting_recharge_key || recharge_bind_type != CFG::shifting_recharge_key_mode)
                                {
                                    recharge_key = CFG::shifting_recharge_key;
                                    recharge_bind_type = CFG::shifting_recharge_key_mode;
                                }
                                gui::keybind("##RechargeKey", &recharge_key, &recharge_bind_type);
                                CFG::shifting_recharge_key = recharge_key;
                                CFG::shifting_recharge_key_mode = recharge_bind_type;
                                ImGui::EndGroup();
                                ImGui::BeginGroup();
                                gui::checkbox("Shifting Warp", CFG::shifting_warp);
                                ImGui::SameLine(350.0f);
                                static int warp_key = CFG::shifting_warp_key;
                                static int warp_bind_type = CFG::shifting_warp_key_mode;
                                if (warp_key != CFG::shifting_warp_key || warp_bind_type != CFG::shifting_warp_key_mode)
                                {
                                    warp_key = CFG::shifting_warp_key;
                                    warp_bind_type = CFG::shifting_warp_key_mode;
                                }
                                gui::keybind("##WarpKey", &warp_key, &warp_bind_type);
                                CFG::shifting_warp_key = warp_key;
                                CFG::shifting_warp_key_mode = warp_bind_type;
                                ImGui::EndGroup();
                                gui::checkbox("SeedPred", CFG::Exploits_SeedPred_Active);
                                gui::checkbox("No Spread", CFG::Aimbot_Projectile_NoSpread);
                            }
                            gui::end_group();
                        }

                        else if (aimbot_mode == 1) {
                            if (gui::begin_group_scrollable("MAIN", ImVec2(380, 250), 5.0f, 0.0f))
                            {
                                ImGui::BeginGroup();
                                gui::checkbox("Projectile Aimbot", CFG::Aimbot_Projectile_Active);
                                ImGui::SameLine(350.0f);

                                static int projectile_key = CFG::Aimbot_Key;
                                static int projectile_keymode = CFG::Aimbot_Projectile_KeyMode;

                                if (projectile_key != CFG::Aimbot_Key || projectile_keymode != CFG::Aimbot_Projectile_KeyMode)
                                {
                                    projectile_key = CFG::Aimbot_Key;
                                    projectile_keymode = CFG::Aimbot_Projectile_KeyMode;
                                }

                                gui::keybind("##ProjectileKey", &projectile_key, &projectile_keymode);

                                CFG::Aimbot_Key = projectile_key;
                                CFG::Aimbot_Projectile_KeyMode = projectile_keymode;
                                ImGui::EndGroup();

                                // Basic Settings
                                gui::slider("FOV", &CFG::Aimbot_Projectile_FOV, 0.f, 180.f);

                                gui::combo("Aim Type", &CFG::Aimbot_Projectile_Mode,
                                    std::vector<std::string>{ "Plain", "Silent", "Smooth" });

                                if (CFG::Aimbot_Projectile_Mode == 2)
                                {
                                    gui::slider("Smoothing", &CFG::Aimbot_Projectile_Smoothing, 1.f, 20.f);
                                }

                                gui::combo("Sort", &CFG::Aimbot_Projectile_Sort,
                                    std::vector<std::string>{ "FOV", "Distance", "Health" });
                            }
                            gui::end_group_scrollable();

                            // ========== ACCURACY SETTINGS ==========
                            ImGui::SameLine(390);
                            if (gui::begin_group_scrollable("ACCURACY", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                // Advanced Head Aim (NOVO - estava faltando!)
                                gui::checkbox("Advanced Head Aim", CFG::Aimbot_Projectile_Advanced_Head_Aim);
                                gui::checkbox("Splash Bot", CFG::Aimbot_Projectile_SplashBot);
                                if (CFG::Aimbot_Projectile_SplashBot)
                                {
                                    ImGui::Indent(15.0f);
                                    gui::slider("Splash Radius Multiplier", &CFG::Aimbot_Projectile_SplashRadius, 0.5f, 2.0f);
                                    static int splash_points = CFG::Aimbot_Projectile_SplashTestPoints;
                                    gui::slider("Splash Test Points", (float*)&splash_points, 4.f, 16.f);
                                    CFG::Aimbot_Projectile_SplashTestPoints = (int)splash_points;
                                    gui::slider("Splash Max Distance", &CFG::Aimbot_Projectile_SplashMaxDist, 32.0f, 200.0f);
                                    gui::checkbox("Prioritize Ground Shots", CFG::Aimbot_Projectile_SplashPrioritizeGround);
                                    gui::checkbox("Neural Network Prediction", CFG::Aimbot_Projectile_SplashUseNN);
                                    ImGui::Unindent(15.0f);
                                }
                                // Double Donk (CORRIGIDO nome da variável)
                                gui::checkbox("Auto Double Donk", CFG::Aimbot_Projectile_Auto_Double_Donk);
                                gui::slider("Max Simulation Time", &CFG::Aimbot_Projectile_Max_Simulation_Time, 1.0f, 5.0f);
                                static int max_targets = CFG::Aimbot_Projectile_Max_Processing_Targets;
                                gui::slider("Max Targets", (float*)&max_targets, 1.f, 10.f);
                                CFG::Aimbot_Projectile_Max_Processing_Targets = (int)max_targets;
                            }
                            gui::end_group_scrollable();

                            // ========== TARGET SETTINGS ==========
                            if (gui::begin_group_scrollable("TARGET", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                gui::checkbox("Target Players", CFG::Aimbot_Target_Players);
                                gui::checkbox("Target Buildings", CFG::Aimbot_Target_Buildings);
                                gui::checkbox("Target Stickies", CFG::Aimbot_TargetStickies);
                                gui::checkbox("Team Check", CFG::Aimbot_Projectile_TeamCheck);
                                gui::checkbox("Ignore Invisible", CFG::Aimbot_Ignore_Invisible);
                                gui::checkbox("Ignore Invulnerable", CFG::Aimbot_Ignore_Invulnerable);
                                gui::checkbox("Ignore Taunting", CFG::Aimbot_Ignore_Taunting);

                                gui::combo("Aim Position", &CFG::Aimbot_Projectile_AimPosition,
                                    std::vector<std::string>{ "Auto", "Head", "Body", "Feet" });
                            }
                            gui::end_group_scrollable();

                            // ========== EXPLOITS ==========
                            ImGui::SameLine(390);
                            if (gui::begin_group("EXPLOITS", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                ImGui::BeginGroup();
                                gui::checkbox("Shifting", CFG::shifting_active);
                                ImGui::SameLine(350.0f);
                                static int shifting_key = CFG::shifting_key;
                                static int shifting_bind_type = CFG::shifting_key_mode;
                                if (shifting_key != CFG::shifting_key || shifting_bind_type != CFG::shifting_key_mode)
                                {
                                    shifting_key = CFG::shifting_key;
                                    shifting_bind_type = CFG::shifting_key_mode;
                                }
                                gui::keybind("##ShiftingKey", &shifting_key, &shifting_bind_type);
                                CFG::shifting_key = shifting_key;
                                CFG::shifting_key_mode = shifting_bind_type;
                                ImGui::EndGroup();

                                gui::slider("Delay Ticks", &CFG::shifting_delay_ticks, 0.f, 20.f);
                                gui::slider("Delay Hitscan", &CFG::shifting_delay_hitscan, 0.f, 10.f);
                                ImGui::BeginGroup();
                                gui::checkbox("Recharge", CFG::shifting_active);
                                ImGui::SameLine(350.0f);
                                static int recharge_key = CFG::shifting_recharge_key;
                                static int recharge_bind_type = CFG::shifting_recharge_key_mode;
                                if (recharge_key != CFG::shifting_recharge_key || recharge_bind_type != CFG::shifting_recharge_key_mode)
                                {
                                    recharge_key = CFG::shifting_recharge_key;
                                    recharge_bind_type = CFG::shifting_recharge_key_mode;
                                }
                                gui::keybind("##RechargeKey", &recharge_key, &recharge_bind_type);
                                CFG::shifting_recharge_key = recharge_key;
                                CFG::shifting_recharge_key_mode = recharge_bind_type;
                                ImGui::EndGroup();

                                ImGui::BeginGroup();
                                gui::checkbox("Warp", CFG::shifting_warp);
                                ImGui::SameLine(350.0f);
                                static int warp_key = CFG::shifting_warp_key;
                                static int warp_bind_type = CFG::shifting_warp_key_mode;
                                if (warp_key != CFG::shifting_warp_key || warp_bind_type != CFG::shifting_warp_key_mode)
                                {
                                    warp_key = CFG::shifting_warp_key;
                                    warp_bind_type = CFG::shifting_warp_key_mode;
                                }
                                gui::keybind("##WarpKey", &warp_key, &warp_bind_type);
                                CFG::shifting_warp_key = warp_key;
                                CFG::shifting_warp_key_mode = warp_bind_type;
                                ImGui::EndGroup();

                                gui::checkbox("Seed Prediction", CFG::Exploits_SeedPred_Active);
                                gui::checkbox("No Spread", CFG::Aimbot_Projectile_NoSpread);
                            }
                            gui::end_group();
                        }

                        // ===== MELEE MODE (2) =====
                        else if (aimbot_mode == 2) {
                            if (gui::begin_group_scrollable("MELEE SETTINGS", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Melee Aimbot Settings");

                                static bool melee_active = false;
                                gui::checkbox("Enable Melee Aimbot", melee_active);

                                gui::slider("Melee FOV", &CFG::Aimbot_FOV, 0.f, 180.f);

                                static int melee_mode = 0;
                                gui::combo("Melee Mode", &melee_mode,
                                    std::vector<std::string>{"Normal", "Backstab", "Auto-Facestab"});

                                static bool swing_prediction = false;
                                gui::checkbox("Swing Prediction", swing_prediction);

                                static bool auto_backstab = false;
                                gui::checkbox("Auto Backstab", auto_backstab);

                                static bool ignore_razorback = false;
                                gui::checkbox("Ignore Razorback", ignore_razorback);

                                static int melee_sort = 0;
                                gui::combo("Sort Method", &melee_sort,
                                    std::vector<std::string>{"FOV", "Distance", "Health"});
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);
                            if (gui::begin_group_scrollable("MELEE OPTIONS", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Melee Options");

                                gui::checkbox("Target Players", CFG::Aimbot_Target_Players);
                                gui::checkbox("Target Buildings", CFG::Aimbot_Target_Buildings);

                                gui::checkbox("Ignore Invisible", CFG::Aimbot_Ignore_Invisible);
                                gui::checkbox("Ignore Invulnerable", CFG::Aimbot_Ignore_Invulnerable);
                                gui::checkbox("Team Check", CFG::Aimbot_TeamCheck);
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Spy Tools");

                                static bool auto_disguise = false;
                                gui::checkbox("Auto Disguise After Stab", auto_disguise);

                                static bool cloak_warning = false;
                                gui::checkbox("Low Cloak Warning", cloak_warning);

                                static bool backstab_indicator = false;
                                gui::checkbox("Backstab Indicator", backstab_indicator);
                            }
                            gui::end_group_scrollable();
                        }
                    }
                    // ANTI AIM TAB
                    if (active_tab == 1)
                    {
                    }

                    // VISUALS TAB
                    if (active_tab == 2)
                    {
                        int vis_mode = gui::g_visuals_mode_state.selected_mode;

                        // ===== ESP MODE =====
                        if (vis_mode == 0) {
                            if (gui::begin_group_scrollable("PLAYERS ESP", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                gui::checkbox("ESP Master", CFG::ESP_Enable);
                                gui::checkbox("Team Check", CFG::ESP_Team);

                                gui::checkbox_color("Box", CFG::ESP_Box, &CFG::ESP_BoxColor);
                                gui::combo("Box Style", &CFG::ESP_BoxType, std::vector<std::string>{"2D", "3D", "Corner"});

                                gui::checkbox_color("Name", CFG::ESP_Name, &CFG::ESP_NameColor);

                                gui::checkbox_color("Health", CFG::ESP_Health, &CFG::ESP_HealthBarColor);
                                gui::combo("Health Type", &CFG::ESP_HealthType, std::vector<std::string>{"Health bar", "Health number", "Number + bar"});
                                gui::combo("Health Position", &CFG::ESP_HealthBarPosition, std::vector<std::string>{"Left", "Right", "Top", "Bottom"});

                                // Health Bar Gradient - mantém BeginGroup porque tem 3 color pickers na mesma linha
                                ImGui::BeginGroup();
                                gui::checkbox("Health Bar Gradient", CFG::ESP_HealthBarGradient);
                                ImGui::SameLine(310.0f);
                                gui::color_picker("##health_low", &CFG::ESP_HealthBarGradientLow, false);
                                ImGui::SameLine(0.0f, 4.0f);
                                gui::color_picker("##health_mid", &CFG::ESP_HealthBarGradientMid, false);
                                ImGui::SameLine(0.0f, 4.0f);
                                gui::color_picker("##health_high", &CFG::ESP_HealthBarGradientHigh, false);
                                ImGui::EndGroup();

                                gui::checkbox("Show Local Player", CFG::ESP_LocalPlayer);
                                gui::checkbox("Hide Cloaked Players", CFG::ESP_HideCloaked);
                                gui::checkbox("Player Conditions", CFG::ESP_Conds);

                                gui::checkbox_color("Player Tracers", CFG::ESP_Tracer, &CFG::ESP_TracerColor);

                                gui::checkbox("Buffs", CFG::ESP_Buffs);
                                gui::checkbox("Debuffs", CFG::ESP_Debuffs);

                                gui::checkbox_color("Latency (Ping)", CFG::ESP_Ping, &CFG::ESP_PingColor);

                                gui::checkbox_color("Distance Enemy", CFG::ESP_DistanceEnemy, &CFG::ESP_DistanceColor);
                                gui::combo("Distance Position", &CFG::ESP_DistancePosition, std::vector<std::string>{"Side", "Bottom"});
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);
                            if (gui::begin_group_scrollable("WORLD ESP", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                gui::checkbox_color("ESP Build", CFG::ESP_Build, &CFG::ESP_BuildColor, false);
                                gui::checkbox("ESP Build Only Enemy", CFG::ESP_BuildOnlyEnemy);
                                gui::checkbox_color("World Pickups", CFG::ESP_Pickups, &CFG::ESP_PickupsColor, false);
                                gui::checkbox_color("Pickups Box", CFG::ESP_PickupsBox, &CFG::ESP_PickupsBoxColor, false);
                                gui::checkbox_color("Pickups Name", CFG::ESP_PickupsName, &CFG::ESP_PickupsNameColor, false);
                                gui::checkbox("Flag ESP", CFG::ESP_CaptureFlag);
                                gui::checkbox_color("Box Capture", CFG::ESP_BoxCapture, &CFG::ESP_BoxCaptureColor, false);
                                gui::checkbox_color("Name Capture", CFG::ESP_NameCapture, &CFG::ESP_NameCaptureColor, false);
                                gui::checkbox_color("Offscreen Indicators", CFG::ESP_Offscreen, &CFG::ESP_OffscreenColor, false);
                                gui::slider("Offscreen Radius", &CFG::ESP_Offscreen_Radius, 10.f, 500.f);
                                gui::slider("Offscreen Max Distance", &CFG::ESP_Offscreen_MaxDist, 0.f, 2000.f);
                                gui::combo("Offscreen Style", &CFG::ESP_Offscreen_Style, std::vector<std::string>{"Triangle", "Circle", "Bar"});
                                gui::checkbox_color("Offscreen Filled", CFG::ESP_Offscreen_Filled, &CFG::ESP_OffscreenFilledColor, false);
                                gui::checkbox_color("ESP Sniper Lines", CFG::ESP_SniperLines, &CFG::ESP_SniperLinesColor, false);
                                gui::checkbox_color("UberCharge Status", CFG::ESP_Uber, &CFG::ESP_UberStatusColor, false);
                                gui::checkbox_color("UberCharge Bar", CFG::ESP_UberBar, &CFG::ESP_UberBarColor, false);
                                gui::checkbox_color("Aimbot FOV", CFG::Aimbot_DrawFOV, &CFG::Aimbot_FOVColor, false);
                            }
                            gui::end_group_scrollable();

                            if (gui::begin_group_scrollable("INDICATORS", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                std::vector<gui::MultiComboItem> items = {
                                    gui::MultiComboItem("Spectator List", &CFG::Visual_Spectatorlist),
                                    gui::MultiComboItem("Info Painel", &CFG::Indicators_Enable),
                                    gui::MultiComboItem("KeyBind", &CFG::Indicators_Keybinds_Enable),
                                    gui::MultiComboItem("Watermark", &CFG::Indicators_Watermark_Enable),
                                };
                                gui::multi_combo("Indicators", items);

                                // ===== OPÇÕES DO INFO PAINEL =====
                                if (CFG::Indicators_Enable)
                                {
                                    ImGui::Dummy(ImVec2(0, 10));
                                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Info Painel Options:");

                                    std::vector<gui::MultiComboItem> panel_items = {
                                        gui::MultiComboItem("Fake Latency", &CFG::Indicators_Show_FakeLatency),
                                        gui::MultiComboItem("Real Latency", &CFG::Indicators_Show_RealLatency),
                                        gui::MultiComboItem("Scoreboard Latency", &CFG::Indicators_Show_ScoreboardLatency),
                                        gui::MultiComboItem("Inaccuracy", &CFG::Indicators_Show_Inaccuracy),
                                        gui::MultiComboItem("Velocity", &CFG::Indicators_Show_Velocity),
                                    };
                                    gui::multi_combo("Panel Items", panel_items);

                                    gui::combo("Display Mode", &CFG::Indicators_Display_Mode,
                                        std::vector<std::string>{"Bars only", "Numbers only", "Bars + Numbers"});
                                }

                                // ===== OPÇÕES DO WATERMARK =====
                                if (CFG::Indicators_Watermark_Enable)
                                {
                                    ImGui::Dummy(ImVec2(0, 10));
                                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Watermark Options:");

                                    std::vector<gui::MultiComboItem> watermark_items = {
                                        gui::MultiComboItem("Steam Name", &CFG::Watermark_ShowName),
                                        gui::MultiComboItem("FPS", &CFG::Watermark_ShowFPS),
                                        gui::MultiComboItem("Ping", &CFG::Watermark_ShowPing),
                                        gui::MultiComboItem("Time", &CFG::Watermark_ShowTime),
                                    };
                                    gui::multi_combo("Watermark Info", watermark_items);
                                }
                            }
                            gui::end_group_scrollable();
                            ImGui::SameLine(390);
                            if (gui::begin_group_scrollable("SKELETON ESP", ImVec2(380, 250), 5.0f, 5.0f))
                            {
                                gui::checkbox_color("Skeleton", CFG::ESP_Skeleton, &CFG::Color_Skeleton);

                                // Tornar On Shot, On Hit e Aim Points como multi combo
                                std::vector<gui::MultiComboItem> skeleton_items = {
                                    gui::MultiComboItem("On Shot", &CFG::ESP_Skeleton_OnShot),
                                    gui::MultiComboItem("On Hit", &CFG::ESP_Skeleton_OnHit),
                                    gui::MultiComboItem("Aim Points", &CFG::ESP_Skeleton_AimPoints)
                                };
                                gui::multi_combo("Skeleton Options", skeleton_items);
                                if (CFG::ESP_Skeleton_OnShot)
                                    gui::color_picker("##Skeleton_OnShot_Color", &CFG::Color_Skeleton_OnShot, false);
                                if (CFG::ESP_Skeleton_OnHit)
                                    gui::color_picker("##Skeleton_OnHit_Color", &CFG::Color_Skeleton_OnHit, false);
                                if (CFG::ESP_Skeleton_AimPoints)
                                    gui::color_picker("##Skeleton_AimPoints_Color", &CFG::Color_Skeleton_AimPoints, false);
                                gui::checkbox_color("Bounds", CFG::ESP_Skeleton_Bounds, &CFG::Color_Skeleton_Bounds);
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Backtrack Options:");
                                gui::checkbox_color("Backtrack Skeleton", CFG::ESP_Skeleton_Backtrack, &CFG::Color_BacktrackSkeleton);
                                if (CFG::ESP_Skeleton_Backtrack)
                                {
                                    gui::combo("Backtrack Style", &CFG::ESP_Skeleton_BacktrackType,
                                        std::vector<std::string>{"Lines", "Dots"});
                                }
                            }
                            gui::end_group_scrollable();
                        }

                        if (vis_mode == 1) {
                            if (gui::begin_group_scrollable("PLAYER MATERIALS", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Player Chams");

                                gui::checkbox("Chams Master", CFG::Materials_Active);

                                if (CFG::Materials_Active)
                                {
                                    gui::checkbox("Player Chams", CFG::Materials_Players_Active);

                                    if (CFG::Materials_Players_Active)
                                    {
                                        const char* material_types[] = { "None", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel" };
                                        gui::combo("Material Type", &CFG::Materials_Players_Material,
                                            std::vector<std::string>(material_types, material_types + 7));

                                        gui::checkbox("Ignore Depth (Wallhack)", CFG::Materials_Players_IgnoreDepth);
                                        if (ImGui::IsItemHovered())
                                            ImGui::SetTooltip("Renders players through walls");

                                        const char* overlay_types[] = { "None", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel", "Overlay", "Exorcism", };
                                        gui::combo("Overlay Type", &CFG::Materials_Players_TwoModels,
                                            std::vector<std::string>(overlay_types, overlay_types + 11));

                                        // ===== IGNORE LOCAL =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Local", CFG::Materials_Players_Ignore_Local);
                                        if (!CFG::Materials_Players_Ignore_Local)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorLocal", &CFG::Color_Players_Local, true);
                                            ImGui::SameLine(0.0f, 4.0f);
                                            gui::color_picker("##OverlayLocal", &CFG::Color_Players_Overlay_Local, true);
                                        }
                                        ImGui::EndGroup();

                                        // ===== IGNORE TEAMMATES =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Teammates", CFG::Materials_Players_Ignore_Teammates);
                                        if (!CFG::Materials_Players_Ignore_Teammates)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorTeammates", &CFG::Color_Players_Teammates, true);
                                            ImGui::SameLine(0.0f, 4.0f);
                                            gui::color_picker("##OverlayTeammates", &CFG::Color_Players_Overlay_Teammates, true);
                                        }
                                        ImGui::EndGroup();

                                        // ===== IGNORE ENEMIES =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Enemies", CFG::Materials_Players_Ignore_Enemies);
                                        if (!CFG::Materials_Players_Ignore_Enemies)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorEnemies", &CFG::Color_Players_Enemies, true);
                                            ImGui::SameLine(0.0f, 4.0f);
                                            gui::color_picker("##OverlayEnemies", &CFG::Color_Players_Overlay_Enemies, true);
                                        }
                                        ImGui::EndGroup();

                                        // ===== IGNORE FRIENDS =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Friends", CFG::Materials_Players_Ignore_Friends);
                                        if (!CFG::Materials_Players_Ignore_Friends)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorFriends", &CFG::Color_Players_Friends, true);
                                            ImGui::SameLine(0.0f, 4.0f);
                                            gui::color_picker("##OverlayFriends", &CFG::Color_Players_Overlay_Friends, true);
                                        }
                                        ImGui::EndGroup();

                                        // ===== IGNORE LAG RECORDS =====
                                        gui::checkbox("Ignore Lag Records", CFG::Materials_Players_Ignore_LagRecords);
                                        if (!CFG::Materials_Players_Ignore_LagRecords)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorLagRecords", &CFG::Color_Players_LagRecords, true);
                                            ImGui::Spacing();
                                            const char* lagrecord_styles[] = { "Flat", "Shaded" };
                                            gui::combo("", &CFG::Materials_Players_LagRecords_Style,
                                                std::vector<std::string>(lagrecord_styles, lagrecord_styles + 2));
                                        }
                                    }
                                }
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);
                            if (gui::begin_group_scrollable("BUILDING MATERIALS", ImVec2(380, 500), 5.0f, 5.0f))
                            {

                                if (CFG::Materials_Active)
                                {
                                    gui::checkbox("Building Chams", CFG::Materials_Buildings_Active);

                                    if (CFG::Materials_Buildings_Active)
                                    {
                                        const char* material_types[] = { "None", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel" };
                                        gui::combo("Material Type", &CFG::Materials_Buildings_Material,
                                            std::vector<std::string>(material_types, material_types + 7));

                                        gui::checkbox("Ignore Depth (Wallhack)", CFG::Materials_Buildings_IgnoreDepth);
                                        if (ImGui::IsItemHovered())
                                            ImGui::SetTooltip("Renders buildings through walls");

                                        const char* overlay_types[] = { "None", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel", "Overlay", "Killstreak", "Exorcism", "Flat Overlay" };
                                        gui::combo("Overlay Type", &CFG::Materials_Buildings_TwoModels,
                                            std::vector<std::string>(overlay_types, overlay_types + 11));

                                        // ===== IGNORE LOCAL =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Local", CFG::Materials_Buildings_Ignore_Local);
                                        if (!CFG::Materials_Buildings_Ignore_Local)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorBuildingsLocal", &CFG::Color_Buildings_Local, true);
                                            if (CFG::Materials_Buildings_TwoModels > 0)
                                            {
                                                ImGui::SameLine(0.0f, 4.0f);
                                                gui::color_picker("##OverlayBuildingsLocal", &CFG::Color_Buildings_Overlay_Local, true);
                                            }
                                        }
                                        ImGui::EndGroup();

                                        // ===== IGNORE TEAMMATES =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Teammates", CFG::Materials_Buildings_Ignore_Teammates);
                                        if (!CFG::Materials_Buildings_Ignore_Teammates)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorBuildingsTeammates", &CFG::Color_Buildings_Teammates, true);
                                            if (CFG::Materials_Buildings_TwoModels > 0)
                                            {
                                                ImGui::SameLine(0.0f, 4.0f);
                                                gui::color_picker("##OverlayBuildingsTeammates", &CFG::Color_Buildings_Overlay_Teammates, true);
                                            }
                                        }
                                        ImGui::EndGroup();

                                        if (!CFG::Materials_Buildings_Ignore_Teammates)
                                        {
                                            ImGui::Indent(15.0f);
                                            gui::checkbox("Show Teammate Dispensers", CFG::Materials_Buildings_Show_Teammate_Dispensers);
                                            ImGui::Unindent(15.0f);
                                        }

                                        // ===== IGNORE ENEMIES =====
                                        ImGui::BeginGroup();
                                        gui::checkbox("Ignore Enemies", CFG::Materials_Buildings_Ignore_Enemies);
                                        if (!CFG::Materials_Buildings_Ignore_Enemies)
                                        {
                                            ImGui::SameLine(280.0f);
                                            gui::color_picker("##ColorBuildingsEnemies", &CFG::Color_Buildings_Enemies, true);
                                            if (CFG::Materials_Buildings_TwoModels > 0)
                                            {
                                                ImGui::SameLine(0.0f, 4.0f);
                                                gui::color_picker("##OverlayBuildingsEnemies", &CFG::Color_Buildings_Overlay_Enemies, true);
                                            }
                                        }
                                        ImGui::EndGroup();
                                    }
                                }
                                else
                                {
                                    ImGui::Spacing();
                                    ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.f), "Enable Chams Master first!");
                                }
                            }
                            gui::end_group_scrollable();
                        }

                        // ===== WORLD MODE =====
                        else if (vis_mode == 2) {
                            if (gui::begin_group_scrollable("WORLD SETTINGS", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "World Modifications");
                                // Adicione configurações de world
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);
                            if (gui::begin_group_scrollable("PARTICLES", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "Particle Effects");
                                // Adicione configurações de particles
                            }
                            gui::end_group_scrollable();
                        }
                    }

                    // PLAYERS TAB
                    if (active_tab == 3)
                    {
                        if (gui::begin_group_scrollable("PLAYERS IN SERVER", ImVec2(380, 500), 5.0f, 5.0f))
                        {
                            int playerCount = 0;
                            for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
                            {
                                if (i == I::EngineClient->GetLocalPlayer())
                                    continue;

                                player_info_t pi{};
                                if (!I::EngineClient->GetPlayerInfo(i, &pi) || pi.fakeplayer)
                                    continue;

                                auto pEntity = I::ClientEntityList->GetClientEntity(i);
                                if (!pEntity)
                                    continue;

                                auto pBaseEntity = pEntity->As<C_BaseEntity>();
                                if (!pBaseEntity)
                                    continue;

                                auto pPlayer = pEntity->As<C_TFPlayer>();
                                if (!pPlayer)
                                    continue;

                                playerCount++;

                                PlayerPriority priority{};
                                F::Players->GetInfo(i, priority);

                                ImGui::PushID(i);

                                ImVec4 nameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                                if (priority.Cheater)
                                    nameColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                                else if (priority.RetardLegit)
                                    nameColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
                                else if (priority.CheaterLight)
                                    nameColor = ImVec4(1.0f, 0.6f, 0.6f, 1.0f);
                                else if (priority.Suspect)
                                    nameColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                                else if (priority.RijinUser)
                                    nameColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
                                else if (priority.LmaoboxUser)
                                    nameColor = ImVec4(1.0f, 0.7f, 0.3f, 1.0f);
                                else if (priority.NethookUser)
                                    nameColor = ImVec4(0.5f, 0.0f, 0.5f, 1.0f);
                                else if (priority.Ignored)
                                    nameColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

                                int team = pBaseEntity->m_iTeamNum();
                                const char* teamTag = team == 2 ? "[RED]" : team == 3 ? "[BLU]" : "[SPEC]";
                                ImVec4 teamColor = team == 2 ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) :
                                    team == 3 ? ImVec4(0.2f, 0.5f, 1.0f, 1.0f) :
                                    ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

                                ImGui::TextColored(teamColor, "%s", teamTag);
                                ImGui::SameLine();
                                ImGui::PushStyleColor(ImGuiCol_Text, nameColor);
                                ImGui::Selectable(pi.name, false, 0, ImVec2(320.0f, 0));
                                ImGui::PopStyleColor();

                                if (ImGui::BeginPopupContextItem())
                                {
                                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                                    ImDrawList* draw = ImGui::GetWindowDrawList();
                                    ImVec2 popup_pos = ImGui::GetWindowPos();
                                    ImVec2 popup_size(284, 165);
                                    ImVec2 header_size(284, 35);

                                    int grid_size = 5;
                                    int rows = (int)((popup_size.y - header_size.y) / grid_size);
                                    int cols = (int)(popup_size.x / grid_size);

                                    for (int io = 0; io < cols; io++)
                                    {
                                        for (int jo = 0; jo < rows; jo++)
                                        {
                                            float x = popup_pos.x + (io * grid_size);
                                            float y = popup_pos.y + header_size.y + (jo * grid_size);
                                            ImU32 color;
                                            if ((io + jo) % 2 == 0)
                                                color = IM_COL32(20, 20, 20, 255);
                                            else
                                                color = IM_COL32(25, 25, 25, 255);
                                            draw->AddRectFilled(ImVec2(x, y), ImVec2(x + grid_size, y + grid_size), color);
                                        }
                                    }

                                    ImVec2 header_pos = popup_pos;
                                    ImU32 accent_faded = modify_theme ? IM_COL32(theme_color.r, theme_color.g, theme_color.b, 70) : IM_COL32(0, 122, 187, 70);
                                    ImU32 accent_main = modify_theme ? IM_COL32(theme_color.r, theme_color.g, theme_color.b, theme_color.a) : IM_COL32(0, 122, 187, 255);
                                    draw->AddRectFilledMultiColor(
                                        header_pos,
                                        ImVec2(header_pos.x + header_size.x, header_pos.y + header_size.y),
                                        accent_faded,
                                        accent_faded,
                                        accent_main,
                                        accent_main
                                    );

                                    ImVec2 text_size = ImGui::CalcTextSize(pi.name);
                                    ImVec2 text_pos(
                                        header_pos.x + (header_size.x - text_size.x) * 0.5f,
                                        header_pos.y + (header_size.y - text_size.y) * 0.5f
                                    );

                                    draw->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), IM_COL32(10, 10, 10, 200), pi.name);
                                    draw->AddText(text_pos, IM_COL32(240, 240, 240, 255), pi.name);
                                    draw->AddRect(popup_pos, ImVec2(popup_pos.x + popup_size.x, popup_pos.y + popup_size.y), IM_COL32(40, 40, 40, 255));

                                    ImGui::SetCursorPos(ImVec2(0, header_size.y));
                                    ImGui::BeginChild("##buttons", ImVec2(284, 130), false, ImGuiWindowFlags_NoScrollbar);

                                    const float btnWidth = 82.0f;
                                    const float btnHeight = 32.0f;
                                    const ImVec2 btnSize(btnWidth, btnHeight);
                                    const float startX = 12.0f;
                                    const float startY = 12.0f;
                                    const float spacingX = 6.0f;
                                    const float spacingY = 6.0f;

                                    auto draw_flag_button = [&](const char* label, bool active, ImVec4 activeColor, float x, float y) -> bool {
                                        ImGui::SetCursorPos(ImVec2(x, y));
                                        ImVec2 pos = ImGui::GetCursorScreenPos();
                                        bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y));
                                        bool clicked = hovered && ImGui::IsMouseClicked(0);

                                        ImU32 bg_top, bg_bottom;
                                        if (active) {
                                            bg_top = IM_COL32(52, 52, 52, 255);
                                            bg_bottom = IM_COL32(41, 41, 41, 255);
                                        }
                                        else if (hovered) {
                                            bg_top = IM_COL32(35, 35, 35, 255);
                                            bg_bottom = IM_COL32(25, 25, 25, 255);
                                        }
                                        else {
                                            bg_top = IM_COL32(43, 43, 43, 255);
                                            bg_bottom = IM_COL32(33, 33, 33, 255);
                                        }

                                        draw->AddRectFilledMultiColor(
                                            pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y),
                                            bg_top, bg_top, bg_bottom, bg_bottom
                                        );

                                        if (active) {
                                            ImU32 accent_top = ImGui::ColorConvertFloat4ToU32(ImVec4(activeColor.x, activeColor.y, activeColor.z, 0.4f));
                                            ImU32 accent_bottom = ImGui::ColorConvertFloat4ToU32(ImVec4(activeColor.x * 0.5f, activeColor.y * 0.5f, activeColor.z * 0.5f, 0.6f));
                                            draw->AddRectFilledMultiColor(
                                                pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y),
                                                accent_top, accent_top, accent_bottom, accent_bottom
                                            );
                                        }

                                        ImU32 border_color;
                                        if (active) {
                                            border_color = ImGui::ColorConvertFloat4ToU32(activeColor);
                                        }
                                        else {
                                            border_color = IM_COL32(15, 15, 15, 155);
                                        }
                                        draw->AddRect(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y), border_color, 0.0f, 0, 1.0f);

                                        ImVec2 label_size = ImGui::CalcTextSize(label);
                                        ImVec2 label_pos(
                                            pos.x + (btnSize.x - label_size.x) * 0.5f,
                                            pos.y + (btnSize.y - label_size.y) * 0.5f
                                        );

                                        draw->AddText(ImVec2(label_pos.x + 1, label_pos.y + 1), IM_COL32(10, 10, 10, 150), label);

                                        ImU32 text_color;
                                        if (active) {
                                            text_color = ImGui::ColorConvertFloat4ToU32(activeColor);
                                        }
                                        else if (hovered) {
                                            text_color = IM_COL32(220, 220, 220, 255);
                                        }
                                        else {
                                            text_color = IM_COL32(180, 180, 180, 255);
                                        }
                                        draw->AddText(label_pos, text_color, label);

                                        ImGui::Dummy(btnSize);
                                        return clicked;
                                        };

                                    float col0 = startX;
                                    float col1 = startX + btnWidth + spacingX;
                                    float col2 = startX + (btnWidth + spacingX) * 2;
                                    float row0 = startY;
                                    float row1 = startY + btnHeight + spacingY;
                                    float row2 = startY + (btnHeight + spacingY) * 2;

                                    if (draw_flag_button("Cheater", priority.Cheater, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), col0, row0)) {
                                        priority = {}; priority.Cheater = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Retard", priority.RetardLegit, ImVec4(1.0f, 0.5f, 0.0f, 1.0f), col1, row0)) {
                                        priority = {}; priority.RetardLegit = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Light", priority.CheaterLight, ImVec4(1.0f, 0.6f, 0.6f, 1.0f), col2, row0)) {
                                        priority = {}; priority.CheaterLight = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Suspect", priority.Suspect, ImVec4(1.0f, 1.0f, 0.0f, 1.0f), col0, row1)) {
                                        priority = {}; priority.Suspect = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Rijin", priority.RijinUser, ImVec4(0.0f, 0.5f, 1.0f, 1.0f), col1, row1)) {
                                        priority = {}; priority.RijinUser = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Lmaobox", priority.LmaoboxUser, ImVec4(1.0f, 0.7f, 0.3f, 1.0f), col2, row1)) {
                                        priority = {}; priority.LmaoboxUser = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Nethook", priority.NethookUser, ImVec4(0.5f, 0.0f, 0.5f, 1.0f), col0, row2)) {
                                        priority = {}; priority.NethookUser = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Ignored", priority.Ignored, ImVec4(0.5f, 0.5f, 0.5f, 1.0f), col1, row2)) {
                                        priority = {}; priority.Ignored = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    if (draw_flag_button("Clear All", false, ImVec4(0.4f, 0.4f, 0.4f, 1.0f), col2, row2)) {
                                        F::Players->Mark(i, PlayerPriority{});
                                        ImGui::CloseCurrentPopup();
                                    }

                                    ImGui::EndChild();
                                    ImGui::PopStyleVar();
                                    ImGui::EndPopup();
                                }
                                ImGui::PopID();
                            }

                            if (playerCount > 0)
                            {
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Total Players: %d", playerCount);
                            }
                        }
                        gui::end_group_scrollable();

                        ImGui::SameLine(390);
                        if (gui::begin_group_scrollable("LEGEND & INFO", ImVec2(380, 500), 5.0f, 5.0f))
                        {
                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "PLAYER FLAGS");
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Clean");
                            ImGui::SameLine(80); ImGui::TextDisabled("No flags");
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Cheater");
                            ImGui::SameLine(80); ImGui::TextDisabled("Confirmed cheater");
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Retard");
                            ImGui::SameLine(80); ImGui::TextDisabled("Suspicious legit");
                            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "Light");
                            ImGui::SameLine(80); ImGui::TextDisabled("Soft cheats");
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Suspect");
                            ImGui::SameLine(80); ImGui::TextDisabled("Under observation");
                            ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f), "Rijin");
                            ImGui::SameLine(80); ImGui::TextDisabled("Rijin cheat");
                            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Lmaobox");
                            ImGui::SameLine(80); ImGui::TextDisabled("Lmaobox cheat");
                            ImGui::TextColored(ImVec4(0.5f, 0.0f, 0.5f, 1.0f), "Nethook");
                            ImGui::SameLine(80); ImGui::TextDisabled("Nethook cheat");
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Ignored");
                            ImGui::SameLine(80); ImGui::TextDisabled("Player ignored");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "HOW TO USE");
                            ImGui::TextWrapped("Right-click players to assign flags. Manual flags persist between sessions. Use 'Clear All' to remove flags.");
                        }
                        gui::end_group_scrollable();
                    }

                    if (active_tab == 4) // SK
                    {
                        ImGui::BeginGroup();
                        {
                            // COLUNA ESQUERDA - War Paints
                            if (gui::begin_group_scrollable("WAR PAINTS & ATTRIBUTES", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                int current_weapon = g_SkinChanger.GetWeaponIndex();
                                bool has_weapon = (current_weapon != -1);

                                if (has_weapon)
                                {
                                    // Lista de War Paints
                                    static const struct {
                                        const char* name;
                                        int id;
                                    } war_paints[] = {
                                        {"None", 0},
                                        // Jungle Jackpot Collection (IDs around 700-710)
                                        {"Park Pigmented", 700},
                                        {"Sax Waxed", 701},
                                        {"Yeti Coated", 702},
                                        {"Croc Dusted", 703},
                                        {"Macaw Masked", 704},
                                        {"Piña Polished", 705},
                                        {"Anodized Aloha", 706},
                                        {"Bamboo Brushed", 707},
                                        {"Leopard Printed", 708},
                                        {"Mannana Peeled", 709},
                                        {"Tiger Buffed", 710},
                                        // Infernal Reward Collection (IDs around 711-723)
                                        {"Fire Glazed", 711},
                                        {"Bonk Varnished", 712},
                                        {"Dream Piped", 713},
                                        {"Freedom Wrapped", 714},
                                        {"Bank Rolled", 715},
                                        {"Clover Camo'd", 716},
                                        {"Kill Covered", 717},
                                        {"Pizza Polished", 718},
                                        {"Bloom Buffed", 719},
                                        {"Cardboard Boxed", 720},
                                        {"Merc Stained", 721},
                                        {"Quack Canvassed", 722},
                                        {"Star Crossed", 723},
                                        // Decorated War Hero Collection (IDs around 390-399)
                                        {"Carpet Bomber Mk.II", 397},
                                        {"Woodland Warrior Mk.II", 390},
                                        {"Wrapped Reviver Mk.II", 398},
                                        {"Forest Fire Mk.II", 391},
                                        {"Night Owl Mk.II", 394},
                                        {"Woodsy Widowmaker Mk.II", 392},
                                        {"Autumn Mk.II", 416},
                                        {"Plaid Potshotter Mk.II", 392},
                                        {"Civil Servant Mk.II", 399},
                                        {"Civic Duty Mk.II", 393},
                                        // Contract Campaigner Collection (IDs around 600-610)
                                        {"Bovine Blazemaker Mk.II", 391},
                                        {"Dead Reckoner Mk.II", 605},
                                        {"Backwoods Boomstick Mk.II", 393},
                                        {"Masked Mender Mk.II", 399},
                                        {"Iron Wood Mk.II", 390},
                                        {"Macabre Web Mk.II", 408},
                                        {"Nutcracker Mk.II", 409},
                                        {"Smalltown Bringdown Mk.II", 419},
                                        // Saxton Select Collection
                                        {"Dragon Slayer", 800},
                                        // Winter 2017 Collection (IDs around 420-434)
                                        {"Miami Element", 420},
                                        {"Jazzy", 421},
                                        {"Mosaic", 422},
                                        {"Cosmic Calamity", 423},
                                        {"Hana", 424},
                                        {"Neo Tokyo", 425},
                                        {"Uranium", 426},
                                        {"Alien Tech", 427},
                                        {"Bomber Soul", 428},
                                        {"Cabin Fevered", 429},
                                        {"Damascus and Mahogany", 430},
                                        {"Dovetailed", 431},
                                        {"Geometrical Teams", 432},
                                        {"Hazard Warning", 433},
                                        {"Polar Surprise", 434},
                                        // Scream Fortress X Collection (IDs around 435-445)
                                        {"Electroshocked", 435},
                                        {"Ghost Town", 436},
                                        {"Tumor Toasted", 437},
                                        {"Calavera Canvas", 438},
                                        {"Spectral Shimmered", 439},
                                        {"Skull Study", 440},
                                        {"Haunted Ghosts", 441},
                                        {"Horror Holiday", 442},
                                        {"Spirit of Halloween", 443},
                                        {"Totally Boned", 444},
                                        // Winter 2019 Collection (IDs around 445-455)
                                        {"Winterland Wrapped", 445},
                                        {"Smissmas Camo", 446},
                                        {"Smissmas Village", 447},
                                        {"Frost Ornamented", 448},
                                        {"Sleighin' Style", 449},
                                        {"Snow Covered", 450},
                                        {"Alpine", 451},
                                        {"Gift Wrapped", 452},
                                        {"Igloo", 453},
                                        {"Seriously Snowed", 454},
                                        // Scream Fortress XII Collection (IDs around 455-470)
                                        {"Spectrum Splattered", 455},
                                        {"Pumpkin Pied", 456},
                                        {"Mummified Mimic", 457},
                                        {"Helldriver", 458},
                                        {"Sweet Toothed", 459},
                                        {"Crawlspace Critters", 460},
                                        {"Raving Dead", 461},
                                        {"Spider's Cluster", 462},
                                        {"Candy Coated", 463},
                                        {"Portal Plastered", 464},
                                        {"Death Deluxe", 465},
                                        {"Eyestalker", 466},
                                        {"Gourdy Green", 467},
                                        {"Spider Season", 468},
                                        {"Organ-ically Hellraised", 469},
                                        // Winter 2020 Collection (IDs around 470-482)
                                        {"Starlight Serenity", 470},
                                        {"Saccharine Striped", 471},
                                        {"Frosty Delivery", 472},
                                        {"Cookie Fortress", 473},
                                        {"Frozen Aurora", 474},
                                        {"Elfin Enamel", 475},
                                        {"Smissmas Spycrabs", 476},
                                        {"Gingerbread Winner", 477},
                                        {"Peppermint Swirl", 478},
                                        {"Gifting Mann's Wrapping Paper", 479},
                                        {"Glacial Glazed", 480},
                                        {"Snow Globalization", 481},
                                        {"Snowflake Swirled", 482},
                                        // Scream Fortress XIII Collection (IDs around 483-495)
                                        {"Misfortunate", 483},
                                        {"Broken Bones", 484},
                                        {"Party Phantoms", 485},
                                        {"Necromanced", 486},
                                        {"Neon-ween", 487},
                                        {"Polter-Guised", 488},
                                        {"Swashbuckled", 489},
                                        {"Kiln and Conquer", 490},
                                        {"Potent Poison", 491},
                                        {"Sarsaparilla Sprayed", 492},
                                        {"Searing Souls", 493},
                                        {"Simple Spirits", 494},
                                        {"Skull Cracked", 495},
                                        // Scream Fortress XIV Collection (IDs around 496-506)
                                        {"Sacred Slayer", 496},
                                        {"Bonzo Gnawed", 497},
                                        {"Ghoul Blaster", 498},
                                        {"Metalized Soul", 499},
                                        {"Pumpkin Plastered", 500},
                                        {"Chilly Autumn", 501},
                                        {"Sunriser", 502},
                                        {"Health and Hell", 503},
                                        {"Health and Hell (Green)", 504},
                                        {"Hypergon", 505},
                                        {"Cream Corned", 506},
                                        // Summer 2023 Collection (IDs around 507-517)
                                        {"Sky Stallion", 507},
                                        {"Business Class", 508},
                                        {"Deadly Dragon", 509},
                                        {"Mechanized Monster", 510},
                                        {"Steel Brushed", 511},
                                        {"Warborn", 512},
                                        {"Bomb Carrier", 513},
                                        {"Pacific Peacemaker", 514},
                                        {"Secretly Serviced", 515},
                                        {"Team Serviced", 516},
                                        // Scream Fortress XVI Collection (IDs around 517-528)
                                        {"Broken Record", 517},
                                        {"Necropolish", 518},
                                        {"Stardust", 519},
                                        {"Graphite Gripped", 520},
                                        {"Piranha Mania", 521},
                                        {"Stealth Specialist", 522},
                                        {"Blackout", 523},
                                        {"Brawler's Iron", 524},
                                        {"Gobi Glazed", 525},
                                        {"Sleek Greek", 526},
                                        {"Team Charged", 527},
                                        {"Team Detail", 528},
                                    };

                                    static int selected_paint = 0;
                                    static int last_clicked_paint = -1;
                                    static float last_click_time = 0.0f;
                                    const float double_click_threshold = 0.3f; // 300ms para duplo clique

                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("Weapon ID: %d", current_weapon);
                                    ImGui::PopStyleColor();

                                    ImGui::Spacing();

                                    // LISTBOX COM DUPLO CLIQUE
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("Double-click to apply:");
                                    ImGui::PopStyleColor();

                                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(17, 17, 17, 255));
                                    ImGui::PushStyleColor(ImGuiCol_Header, gui::GetAccentColor(180));
                                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, gui::GetAccentColor(200));
                                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, gui::GetAccentColor(255));

                                    ImGui::BeginChild("##warpaint_list", ImVec2(360.0f, 300.0f), true, ImGuiWindowFlags_NoScrollWithMouse);

                                    for (int i = 0; i < IM_ARRAYSIZE(war_paints); i++)
                                    {
                                        bool is_selected = (selected_paint == i);

                                        if (ImGui::Selectable(war_paints[i].name, is_selected))
                                        {
                                            float current_time = (float)ImGui::GetTime();

                                            // Detectar duplo clique
                                            if (last_clicked_paint == i && (current_time - last_click_time) < double_click_threshold)
                                            {
                                                // DUPLO CLIQUE - Aplicar a skin
                                                g_SkinChanger.SetAttribute(current_weapon, "paintkit_proto_def_index", static_cast<float>(war_paints[i].id));
                                                I::ClientState->ForceFullUpdate();

                                                // Reset do duplo clique
                                                last_clicked_paint = -1;
                                                last_click_time = 0.0f;
                                            }
                                            else
                                            {
                                                // PRIMEIRO CLIQUE - Apenas selecionar
                                                selected_paint = i;
                                                last_clicked_paint = i;
                                                last_click_time = current_time;
                                            }
                                        }

                                        if (is_selected)
                                            ImGui::SetItemDefaultFocus();
                                    }

                                    ImGui::EndChild();
                                    ImGui::PopStyleColor(4);

                                    ImGui::Spacing();

                                    // SEED E WEAR
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("Seed");
                                    ImGui::PopStyleColor();

                                    static int seed = 0;
                                    ImGui::PushItemWidth(360.0f);
                                    if (ImGui::InputInt("##seed", &seed))
                                    {
                                        if (seed < 0) seed = 0;
                                        if (seed > 9999) seed = 9999;
                                    }
                                    ImGui::PopItemWidth();

                                    ImGui::Spacing();

                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("Wear (0.00 - 1.00)");
                                    ImGui::PopStyleColor();

                                    static float wear = 0.01f;
                                    ImGui::PushItemWidth(360.0f);
                                    if (ImGui::SliderFloat("##wear", &wear, 0.0f, 1.0f, "%.2f"))
                                    {
                                        g_SkinChanger.SetAttribute(current_weapon, "set_item_texture_wear", wear);
                                    }
                                    ImGui::PopItemWidth();

                                    ImGui::Spacing();
                                    ImGui::Spacing();

                                    // BOTÃO FORCE UPDATE
                                    if (gui::button("Force Update", ImVec2(360, 30)))
                                    {
                                        I::ClientState->ForceFullUpdate();
                                    }
                                }
                                else
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255));
                                    const char* text = "Equip a weapon to customize";
                                    float text_width = ImGui::CalcTextSize(text).x;
                                    ImGui::SetCursorPosX((360 - text_width) * 0.5f);
                                    ImGui::SetCursorPosY(220);
                                    ImGui::Text("%s", text);
                                    ImGui::PopStyleColor();
                                }
                            }
                            gui::end_group_scrollable();

                            ImGui::SameLine(390);

                            // COLUNA DIREITA - Killstreaks & Effects
                            if (gui::begin_group_scrollable("KILLSTREAKS & EFFECTS", ImVec2(380, 500), 5.0f, 5.0f))
                            {
                                int current_weapon = g_SkinChanger.GetWeaponIndex();
                                bool has_weapon = (current_weapon != -1);

                                if (has_weapon)
                                {
                                    // KILLSTREAKS
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("KILLSTREAKS");
                                    ImGui::PopStyleColor();
                                    ImGui::Separator();
                                    ImGui::Spacing();

                                    static int ks_tier = 0;
                                    static int ks_sheen = 0;
                                    static int ks_effect = 0;

                                    const char* tiers[] = { "None", "Basic", "Specialized", "Professional" };
                                    const char* sheens[] = { "Team Shine", "Deadly Daffodil", "Manndarin", "Mean Green", "Agonizing Emerald", "Hot Rod", "Villainous Violet" };
                                    const char* effects[] = { "Fire Horns", "Cerebral Discharge", "Tornado", "Flames", "Singularity", "Incinerator", "Hypno-Beam" };

                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("Tier");
                                    ImGui::PopStyleColor();

                                    gui::combo("##ks_tier", &ks_tier, tiers, IM_ARRAYSIZE(tiers), 360.0f);

                                    if (ks_tier >= 2)
                                    {
                                        ImGui::Spacing();
                                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                        ImGui::Text("Sheen");
                                        ImGui::PopStyleColor();
                                        gui::combo("##ks_sheen", &ks_sheen, sheens, IM_ARRAYSIZE(sheens), 360.0f);
                                    }

                                    if (ks_tier >= 3)
                                    {
                                        ImGui::Spacing();
                                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                        ImGui::Text("Effect");
                                        ImGui::PopStyleColor();
                                        gui::combo("##ks_effect", &ks_effect, effects, IM_ARRAYSIZE(effects), 360.0f);
                                    }

                                    ImGui::Spacing();

                                    if (gui::button("Apply Killstreak", ImVec2(360, 30)))
                                    {
                                        if (ks_tier == 0)
                                        {
                                            g_SkinChanger.RemoveAttribute(current_weapon, "killstreak_tier");
                                            g_SkinChanger.RemoveAttribute(current_weapon, "killstreak_effect");
                                            g_SkinChanger.RemoveAttribute(current_weapon, "killstreak_idleeffect");
                                        }
                                        else
                                        {
                                            g_SkinChanger.SetAttribute(current_weapon, "killstreak_tier", static_cast<float>(ks_tier));
                                            if (ks_tier >= 2)
                                            {
                                                g_SkinChanger.SetAttribute(current_weapon, "killstreak_idleeffect", static_cast<float>(ks_sheen + 1));
                                            }
                                            if (ks_tier >= 3)
                                            {
                                                g_SkinChanger.SetAttribute(current_weapon, "killstreak_effect", static_cast<float>(ks_effect + 2002));
                                            }
                                        }
                                        I::ClientState->ForceFullUpdate();
                                    }

                                    ImGui::Spacing();
                                    ImGui::Spacing();

                                    // HALLOWEEN EFFECTS
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("HALLOWEEN EFFECTS");
                                    ImGui::PopStyleColor();
                                    ImGui::Separator();
                                    ImGui::Spacing();

                                    static bool pumpkin_explosions = false;
                                    static bool green_flames = false;
                                    static bool voice_mod = false;
                                    static bool jingle_footsteps = false;

                                    if (gui::checkbox("Pumpkin Explosions", pumpkin_explosions))
                                    {
                                        if (pumpkin_explosions)
                                            g_SkinChanger.SetAttribute(current_weapon, "halloween_pumpkin_explosions", 1.0f);
                                        else
                                            g_SkinChanger.RemoveAttribute(current_weapon, "halloween_pumpkin_explosions");
                                        I::ClientState->ForceFullUpdate();
                                    }

                                    if (gui::checkbox("Green Flames", green_flames))
                                    {
                                        if (green_flames)
                                            g_SkinChanger.SetAttribute(current_weapon, "halloween_green_flames", 1.0f);
                                        else
                                            g_SkinChanger.RemoveAttribute(current_weapon, "halloween_green_flames");
                                        I::ClientState->ForceFullUpdate();
                                    }

                                    if (gui::checkbox("Voice Modulation", voice_mod))
                                    {
                                        if (voice_mod)
                                            g_SkinChanger.SetAttribute(current_weapon, "halloween_voice_modulation", 1.0f);
                                        else
                                            g_SkinChanger.RemoveAttribute(current_weapon, "halloween_voice_modulation");
                                        I::ClientState->ForceFullUpdate();
                                    }

                                    if (gui::checkbox("Jingle Footsteps", jingle_footsteps))
                                    {
                                        if (jingle_footsteps)
                                            g_SkinChanger.SetAttribute(current_weapon, "add_jingle_to_footsteps", 1.0f);
                                        else
                                            g_SkinChanger.RemoveAttribute(current_weapon, "add_jingle_to_footsteps");
                                        I::ClientState->ForceFullUpdate();
                                    }

                                    ImGui::Spacing();
                                    ImGui::Spacing();

                                    // ITEM QUALITY
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                                    ImGui::Text("ITEM QUALITY");
                                    ImGui::PopStyleColor();
                                    ImGui::Separator();
                                    ImGui::Spacing();

                                    static int quality = 0;
                                    const char* qualities[] = {
                                        "Normal", "Genuine", "Vintage", "Unusual", "Unique",
                                        "Community", "Valve", "Self-Made", "Strange", "Haunted", "Collector's"
                                    };

                                    gui::combo("##quality", &quality, qualities, IM_ARRAYSIZE(qualities), 360.0f);

                                    ImGui::Spacing();

                                    if (gui::button("Apply Quality", ImVec2(360, 30)))
                                    {
                                        if (quality > 0)
                                            g_SkinChanger.SetAttribute(current_weapon, "loot_rarity", static_cast<float>(quality));
                                        else
                                            g_SkinChanger.RemoveAttribute(current_weapon, "loot_rarity");
                                        I::ClientState->ForceFullUpdate();
                                    }
                                }
                                else
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255));
                                    const char* text = "Equip a weapon to customize";
                                    float text_width = ImGui::CalcTextSize(text).x;
                                    ImGui::SetCursorPosX((360 - text_width) * 0.5f);
                                    ImGui::SetCursorPosY(220);
                                    ImGui::Text("%s", text);
                                    ImGui::PopStyleColor();
                                }
                            }
                            gui::end_group_scrollable();
                        }
                        ImGui::EndGroup();
                    }

                    // MISC TAB - WITH CONFIG SYSTEM
                    if (active_tab == 5)
                    {
                        ImGui::BeginGroup();
                        {
                            if (gui::begin_group_scrollable("RESTRICTIONS", ImVec2(380, 160), 5.0f, 0.0f))
                            {
                                static bool accord_limit = false;
                                static bool remove_hidden = false;
                                gui::checkbox("Accord limitcheck", accord_limit);
                                gui::checkbox("Remove hidden cmds", remove_hidden);

                                gui::checkbox_color("Modify menu theme", modify_theme, &theme_color, false);

                                // Atualizar o tema se mudou
                                if (modify_theme != old_modify_theme ||
                                    theme_color.r != old_theme_color.r ||
                                    theme_color.g != old_theme_color.g ||
                                    theme_color.b != old_theme_color.b ||
                                    theme_color.a != old_theme_color.a)
                                {
                                    // CORREÇÃO: Usar if/else explícito
                                    if (modify_theme)
                                    {
                                        gui::set_theme(ImVec4(theme_color.r / 255.0f,
                                            theme_color.g / 255.0f,
                                            theme_color.b / 255.0f, 1.0f));
                                    }
                                    else
                                    {
                                        // Quando desmarcar, voltar para o tema padrão
                                        gui::set_theme(ImVec4(0.0f, 122.0f / 255.0f, 187.0f / 255.0f, 1.0f));
                                    }

                                    old_modify_theme = modify_theme;
                                    old_theme_color = theme_color;
                                    CFG::Menu_ModifyTheme = modify_theme;
                                    CFG::Menu_ThemeColor = theme_color;
                                }
                            }
                            gui::end_group_scrollable();

                            if (gui::begin_group_scrollable("MOVEMENT", ImVec2(380, 330), 5.0f, 5.0f))
                            {
                                static bool auto_hop = false;
                                static bool air_duck = false;
                                static bool slide_walk = false;
                                static int autostrafe_mode = 0;

                                gui::checkbox("Auto Bhop", CFG::Misc_AutoJump);
                                gui::checkbox("Air duck", air_duck);
                                gui::checkbox("Slide walk", slide_walk);

                                const char* autostrafe_modes[] = { "None", "Edge jump", "Reverse duck distance", "Fastduck", "Fast walk" };
                                gui::combo("Autostrafe", &autostrafe_mode, autostrafe_modes, 5);

                                ImGui::Spacing();

                                ImDrawList* draw = ImGui::GetWindowDrawList();
                                ImVec2 cursor_start = ImGui::GetCursorScreenPos();
                                float line_height = 22.0f;
                                float label_x = cursor_start.x;
                                float keybind_x = cursor_start.x + 260.0f;

                                draw->AddText(ImVec2(label_x, cursor_start.y), IM_COL32(180, 180, 180, 255), "Edge jump");
                                draw->AddText(ImVec2(keybind_x, cursor_start.y), IM_COL32(150, 150, 150, 255), "[ NONE ]");
                                ImGui::Dummy(ImVec2(0, line_height));

                                cursor_start = ImGui::GetCursorScreenPos();
                                draw->AddText(ImVec2(label_x, cursor_start.y), IM_COL32(180, 180, 180, 255), "Reverse duck distance");
                                draw->AddText(ImVec2(keybind_x, cursor_start.y), IM_COL32(150, 150, 150, 255), "[ NONE ]");
                                ImGui::Dummy(ImVec2(0, line_height));

                                cursor_start = ImGui::GetCursorScreenPos();
                                draw->AddText(ImVec2(label_x, cursor_start.y), IM_COL32(180, 180, 180, 255), "Fastduck");
                                draw->AddText(ImVec2(keybind_x, cursor_start.y), IM_COL32(150, 150, 150, 255), "[ NONE ]");
                                ImGui::Dummy(ImVec2(0, line_height));

                                cursor_start = ImGui::GetCursorScreenPos();
                                draw->AddText(ImVec2(label_x, cursor_start.y), IM_COL32(180, 180, 180, 255), "Fast walk");
                                draw->AddText(ImVec2(keybind_x, cursor_start.y), IM_COL32(150, 150, 150, 255), "[ NONE ]");
                                ImGui::Dummy(ImVec2(0, line_height));
                            }
                            gui::end_group_scrollable();
                        }
                        ImGui::EndGroup();

                        // CONFIG SYSTEM - RIGHT COLUMN
                        ImGui::SameLine(390);
                        if (gui::begin_group_scrollable("CONFIG", ImVec2(380, 500), 5.0f, 5.0f))
                        {
                            gui::listbox("##config_listbox", &selected_config, config_list, 6, 360.0f, 145.0f);

                            ImGui::Spacing();
                            ImGui::Spacing();

                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
                            ImGui::Text("New config name");
                            ImGui::PopStyleColor();

                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 120, 120, 255));
                            ImGui::PushItemWidth(360.0f);
                            ImGui::InputText("##new_config", new_config_name, IM_ARRAYSIZE(new_config_name));
                            ImGui::PopItemWidth();
                            ImGui::PopStyleColor(2);

                            ImGui::Spacing();
                            ImGui::Spacing();

                            // CONFIG BUTTONS WITH FUNCTIONALITY
                            if (gui::button("Refresh", ImVec2(360, 30)))
                            {
                                config_list = Config::RefreshConfigFiles();
                                if (selected_config >= config_list.size())
                                    selected_config = config_list.empty() ? 0 : config_list.size() - 1;
                            }

                            if (gui::button("Create", ImVec2(360, 30)))
                            {
                                if (strlen(new_config_name) > 0)
                                {
                                    Config::CreateConfig(new_config_name);
                                    config_list = Config::RefreshConfigFiles();

                                    for (size_t i = 0; i < config_list.size(); i++)
                                    {
                                        if (config_list[i] == new_config_name)
                                        {
                                            selected_config = i;
                                            break;
                                        }
                                    }

                                    memset(new_config_name, 0, sizeof(new_config_name));
                                }
                            }

                            if (gui::button("Save", ImVec2(360, 30)))
                            {
                                if (!config_list.empty() && selected_config >= 0 && selected_config < config_list.size())
                                {
                                    // Atualizar CFG antes de salvar
                                    CFG::Menu_ModifyTheme = modify_theme;
                                    CFG::Menu_ThemeColor = theme_color;

                                    Config::SaveConfig(config_list[selected_config]);
                                }
                            }

                            if (gui::button("Load", ImVec2(360, 30)))
                            {
                                if (!config_list.empty() && selected_config >= 0 && selected_config < config_list.size())
                                {
                                    Config::LoadConfig(config_list[selected_config]);

                                    // Sincronizar variáveis locais
                                    modify_theme = CFG::Menu_ModifyTheme;
                                    theme_color = CFG::Menu_ThemeColor;
                                    old_modify_theme = modify_theme;
                                    old_theme_color = theme_color;

                                    // Aplicar tema
                                    if (modify_theme)
                                    {
                                        gui::set_theme(ImVec4(theme_color.r / 255.0f,
                                            theme_color.g / 255.0f,
                                            theme_color.b / 255.0f, 1.0f));
                                    }
                                    else
                                    {
                                        gui::set_theme(ImVec4(0.0f, 122.0f / 255.0f, 187.0f / 255.0f, 1.0f));
                                    }
                                }
                            }

                            if (gui::button("Delete", ImVec2(360, 30)))
                            {
                                if (!config_list.empty() && selected_config >= 0 && selected_config < config_list.size())
                                {
                                    Config::DeleteConfig(config_list[selected_config]);
                                    config_list = Config::RefreshConfigFiles();
                                    if (selected_config >= config_list.size())
                                        selected_config = config_list.empty() ? 0 : config_list.size() - 1;
                                }
                            }
                        }
                        gui::end_group_scrollable();
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::End();
        }
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        pStateBlock->Apply();
        pStateBlock->Release();
    }
    RestoreState(pDevice);
    return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (alive && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcId;
    GetWindowThreadProcessId(handle, &wndProcId);
    if (GetCurrentProcessId() != wndProcId)
        return TRUE;
    window = handle;
    return FALSE;
}

HWND GetProcessWindow()
{
    window = NULL;
    EnumWindows(EnumWindowsCallback, NULL);
    return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    hmod = static_cast<HMODULE>(lpReserved);

    App->Start();

    g_SkinChanger.SetPatternScanningMode(true);

    bool attached = false;
    while (!attached && alive)
    {
        if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
        {
            kiero::bind(42, (void**)&oEndScene, hkEndScene);
            while (window == NULL && alive)
            {
                window = GetProcessWindow();
                Sleep(100);
            }
            if (window)
                oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
            attached = true;
        }
        Sleep(100);
    }

    App->Loop();

    while (alive)
    {
        Sleep(100);
    }

    if (window && oWndProc)
        SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)oWndProc);

    kiero::shutdown();

    if (init)
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    if (init)
    {
        if (icon_texture)
        {
            icon_texture->Release();
            icon_texture = nullptr;
        }
    }

    App->Shutdown();
    Sleep(500);
    FreeLibraryAndExitThread(hmod, 0);
    return 0;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        break;
    }
    return TRUE;
}