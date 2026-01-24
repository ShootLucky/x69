#include "includes.h"
#include "App/App.h"
#define STB_IMAGE_IMPLEMENTATION
#include "icons.h"
#include "stb_image.h"
#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif
#include "../nemesis.h"

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
                int grid_size = 5;
                int header_height = 60;
                int rows = (int)((s.y - header_height) / grid_size);
                int cols = (int)(s.x / grid_size);
                for (int io = 0; io < cols; io++)
                {
                    for (int jo = 0; jo < rows; jo++)
                    {
                        float x = p.x + (io * grid_size);
                        float y = p.y + header_height + (jo * grid_size);
                        ImU32 color;
                        if ((io + jo) % 2 == 0)
                            color = IM_COL32(20, 20, 20, 255);
                        else
                            color = IM_COL32(25, 25, 25, 255);
                        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + grid_size, y + grid_size), color);
                    }
                }
                draw_list->AddRect(p, ImVec2(p.x + s.x, p.y + s.y), IM_COL32(40, 40, 40, 255));

                /* tabs */
                ImGui::SetCursorPosX(480);
                ImGui::SetCursorPosY(25);
                ImGui::BeginGroup();
                {
                    gui::TabButton("Aimbot", active_tab, 0);
                    ImGui::SameLine();
                    gui::TabButton("Anti-Aim", active_tab, 1);
                    ImGui::SameLine();
                    gui::TabButton("Visuals", active_tab, 2);
                    ImGui::SameLine();
                    gui::TabButton("Players", active_tab, 3);
                    ImGui::SameLine();
                    gui::TabButton("Misc", active_tab, 4);
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
                        if (gui::begin_group_scrollable("MAIN", ImVec2(380, 250), 5.0f, 0.0f))
                        {
                            ImGui::BeginGroup();
                            gui::checkbox("Aimbot", CFG::Aimbot_Active);
                            ImGui::SameLine(350.0f);
                            static int aimbot_key = CFG::Aimbot_Key;
                            static int aimbot_bind_type = CFG::Aimbot_KeyMode;

                            // Sincronizar variáveis static com CFG (importante para quando carregar configs)
                            if (aimbot_key != CFG::Aimbot_Key || aimbot_bind_type != CFG::Aimbot_KeyMode)
                            {
                                aimbot_key = CFG::Aimbot_Key;
                                aimbot_bind_type = CFG::Aimbot_KeyMode;
                            }

                            gui::keybind("##AimbotKey", &aimbot_key, &aimbot_bind_type);

                            // Atualizar CFG quando o usuário mudar o keybind
                            CFG::Aimbot_Key = aimbot_key;
                            CFG::Aimbot_KeyMode = aimbot_bind_type;
                            ImGui::EndGroup();
                            gui::slider("FOV", &CFG::Aimbot_FOV, 0.f, 180.f);
                            gui::slider("Smoothing", &CFG::Aimbot_Hitscan_Smoothing, 1.f, 20.f);
                            gui::checkbox("Auto Shoot", CFG::Aimbot_AutoShoot);
                            gui::checkbox("Active Lag Records", CFG::Aimbot_ActiveLagRecords);
                            gui::combo("Aim Type", &CFG::Aimbot_Hitscan_Mode, std::vector<std::string>{ "Aimlock", "Silent" });
                            gui::combo("Sort", &CFG::Aimbot_Hitscan_Sort, std::vector<std::string>{ "Distance", "FOV", "Health" });
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

                    // ANTI AIM TAB
                    if (active_tab == 1)
                    {
                    }

                    // VISUALS TAB
                    if (active_tab == 2)
                    {
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
                        if (gui::begin_group("ESP MEDIC", ImVec2(380, 250), 5.0f, 5.0f))
                        {

                        }
                        gui::end_group();
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

                    // MISC TAB - WITH CONFIG SYSTEM
                    if (active_tab == 4)
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

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

    bool attached = false;
    while (!attached && alive)
    {
        if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
        {
            kiero::bind(42, (void**)&oEndScene, hkEndScene);
            while (window == NULL && alive) {
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
        // Libera a textura
        if (icon_texture)
        {
            icon_texture->Release();
            icon_texture = nullptr;
        }

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
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