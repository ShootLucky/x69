#include "includes.h"
#include "App/App.h"
#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif
#include "../nemesis.h"
static int selected_weapon = 0;
static std::vector<bool> hitbox_selected = { false, false, false, false, false };
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

bool init = false;
HMODULE hmod = NULL;

IDirect3DStateBlock9* pStateBlock = NULL;

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX9_Init(pDevice);

    gui::set_theme();
    gui::menu_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\verdanab.ttf", 13.f);;
}

IDirect3DStateBlock9* pixel_state = NULL; IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;
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
                    ImVec2(p.x + s.x, p.y + 60),
                    IM_COL32(24, 24, 24, 255),
                    IM_COL32(24, 24, 24, 255),
                    IM_COL32(16, 16, 16, 255),
                    IM_COL32(16, 16, 16, 255)
                );
                ImU32 accent_color = IM_COL32(0, 122, 187, 255);
                draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + 1), ImVec2(p.x + s.x - 1, p.y + 3), accent_color);
                draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + 60), ImVec2(p.x + s.x - 1, p.y + 62), IM_COL32(40, 40, 40, 255));
                draw_list->AddText(ImVec2(p.x + 6, p.y + 21), IM_COL32(5, 5, 5, 255), "X69 TF2");
                draw_list->AddText(ImVec2(p.x + 5, p.y + 20), accent_color, "X69 TF2");
                draw_list->AddText(ImVec2(p.x + 6, p.y + 33), IM_COL32(5, 5, 5, 255), "DEVELOPED BY");
                draw_list->AddText(ImVec2(p.x + 5, p.y + 32), IM_COL32(255, 255, 255, 100), "DEVELOPED BY");
                float dev_width = ImGui::CalcTextSize("DEVELOPED BY").x;
                draw_list->AddText(ImVec2(p.x + 5 + dev_width + 5, p.y + 33), IM_COL32(5, 5, 5, 255), "SHOOT & VOID");
                draw_list->AddText(ImVec2(p.x + 5 + dev_width + 4, p.y + 32), accent_color, "SHOOT & VOID");
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
                // ImGui::NewLine();
                 // ImGui::NewLine();
                     /* main content */
                ImGui::SetCursorPosX(15);
                ImGui::BeginGroup();
                {
                    // AIMBOT TAB
                    if (active_tab == 0)
                    {
                        if (gui::begin_group_scrollable("MAIN", ImVec2(380, 250), 5.0f, 0.0f))
                        {
                            // Inicia um grupo para a linha do checkbox + keybind (para isolar o layout)
                            ImGui::BeginGroup();
                            gui::checkbox("Aimbot", CFG::Aimbot_Active);
                            ImGui::SameLine(350.0f); // Ajuste este valor para alinhar o keybind à direita (ex: calcule baseado na largura do grupo - largura do keybind)
                            static int aimbot_key = CFG::Aimbot_Key;         // Ou use uma variável do seu config, ex: CFG::Aimbot_K
                            static int aimbot_bind_type = CFG::Aimbot_KeyMode;   // Ou use CFG::Aimbot_BindType (0=Always, 1=Hold on, 2=Toggle, 3=Hold off)
                            gui::keybind("##AimbotKey", &aimbot_key, &aimbot_bind_type);
                            ImGui::EndGroup(); // Fecha o grupo, agora os itens abaixo voltam ao alinhamento normal à esquerda
                            gui::slider("FOV", &CFG::Aimbot_FOV, 0.f, 180.f);
                            gui::slider("Smoothing", &CFG::Aimbot_Hitscan_Smoothing, 0.f, 20.f);
                            gui::checkbox("Visible Check", CFG::Aimbot_VisibleCheck);
                            gui::checkbox("Team Check", CFG::Aimbot_TeamCheck);
                            gui::combo("Aim Type", &CFG::Aimbot_Hitscan_Mode, std::vector<std::string>{ "Aimlock", "Silent" });
                            gui::combo("Sort", &CFG::Aimbot_Hitscan_Sort, std::vector<std::string>{ "Distance", "FOV", "Health" });
                            gui::checkbox("Auto Shoot", CFG::Aimbot_AutoShoot);
                            gui::checkbox("Target Players", CFG::Aimbot_Target_Players);
                            gui::checkbox("Target Buildings", CFG::Aimbot_Target_Buildings);

                        }
                        gui::end_group_scrollable();

                        ImGui::SameLine(390);
                        if (gui::begin_group("ACCURACY", ImVec2(380, 250), 5.0f, 5.0f))
                        {
                            gui::checkbox("Target Lag Records", CFG::Aimbot_TargetLagRecords);
                            gui::checkbox("Target Stickies", CFG::Aimbot_TargetStickies);
                            gui::checkbox("Smooth Auto Shoot", CFG::Aimbot_SmoothAutoShoot);
                            gui::checkbox("Wait For Headshot", CFG::Aimbot_WaitForHeadshot);
                            gui::checkbox("Wait For Charge", CFG::Aimbot_WaitForCharge);
                            gui::checkbox("Minigun Tapfire", CFG::Aimbot_MinigunTapfire);
                            gui::checkbox("Active Lag Records", CFG::Aimbot_ActiveLagRecords);
                            gui::checkbox("Auto Scope", CFG::Aimbot_AutoScope);
                        }
                        gui::end_group();
                        if (gui::begin_group("HITBOXES", ImVec2(380, 250), 5.0f, 5.0f))
                        {
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
                        }
                        gui::end_group();
                        ImGui::SameLine(390);
                        if (gui::begin_group("EXPLOITS", ImVec2(380, 250), 5.0f, 5.0f))
                        {


                        }
                        gui::end_group();
                    }
                    // ANTI AIM TAB
                    if (active_tab == 1)
                    {

                    }
                    if (active_tab == 2)
                    {
                        if (gui::begin_group_scrollable("ESP PLAYERS", ImVec2(380, 250), 5.0f, 5.0f))
                        {
                            gui::checkbox("ESP Master", CFG::ESP_Enable);
                            gui::checkbox("Team Check", CFG::ESP_Team);
                            gui::checkbox("Box", CFG::ESP_Box);
                            gui::combo("Box Style", &CFG::ESP_BoxType, std::vector<std::string>{"2D", "3D", "Corner"});
                            gui::checkbox("Name", CFG::ESP_Name);
                            gui::checkbox("Health", CFG::ESP_Health);
                            gui::combo("Health Type", &CFG::ESP_HealthType, std::vector<std::string>{"Health bar", "Health number", "Number + bar"});
                            gui::checkbox("Show Local Player", CFG::ESP_LocalPlayer);
                            gui::checkbox("Hide Cloaked Players", CFG::ESP_HideCloaked);
                            gui::checkbox("Player Conditions", CFG::ESP_Conds);
                            gui::checkbox("Player Tracers", CFG::ESP_Tracer);
                            gui::checkbox("Buffs", CFG::ESP_Buffs);
                            gui::checkbox("Debuffs", CFG::ESP_Debuffs);
                            gui::checkbox("Latency (Ping)", CFG::ESP_Ping);
                            gui::checkbox("KDR Player", CFG::ESP_KRDPlayer);
                            gui::checkbox("Distance Enemy", CFG::ESP_DistanceEnemy);
                            gui::combo("Distance Position", &CFG::ESP_DistancePosition, std::vector<std::string>{"Side", "Bottom"});
                        }
                        gui::end_group_scrollable();
                        ImGui::SameLine(390);
                        if (gui::begin_group("ESP PICKUPS", ImVec2(380, 250), 5.0f, 5.0f))
                        {
                            gui::checkbox("ESP Build", CFG::ESP_Build);
                            gui::checkbox("ESP Build Only Enemy", CFG::ESP_BuildOnlyEnemy);
                            gui::checkbox("World Pickups", CFG::ESP_Pickups);
                            gui::checkbox("Pickups Box", CFG::ESP_PickupsBox);
                            gui::checkbox("Pickups Name", CFG::ESP_PickupsName);
                            gui::checkbox("Flag ESP", CFG::ESP_CaptureFlag);
                            gui::checkbox("Box Capture", CFG::ESP_BoxCapture);
                            gui::checkbox("Name Capture", CFG::ESP_NameCapture);
                        }
                        gui::end_group();
                        if (gui::begin_group("ESP LINES", ImVec2(380, 250), 5.0f, 5.0f))
                        {
                            gui::checkbox("Offscreen Indicators", CFG::ESP_Offscreen);
                            gui::slider("Offscreen Radius", &CFG::ESP_Offscreen_Radius, 10.f, 500.f);
                            gui::slider("Offscreen Max Distance", &CFG::ESP_Offscreen_MaxDist, 0.f, 2000.f);
                            gui::combo("Offscreen Style", &CFG::ESP_Offscreen_Style, std::vector<std::string>{"Triangle", "Circle", "Bar"});
                            gui::checkbox("Offscreen Filled", CFG::ESP_Offscreen_Filled);
                            gui::checkbox("ESP Sniper Lines", CFG::ESP_SniperLines);
                            gui::checkbox("Lag Compensation", CFG::ESP_LagCompensation);
                        }
                        gui::end_group();
                        ImGui::SameLine(390);
                        if (gui::begin_group("ESP MEDIC", ImVec2(380, 250), 5.0f, 5.0f))
                        {
                            gui::checkbox("UberCharge Status", CFG::ESP_Uber);
                            gui::checkbox("UberCharge Bar", CFG::ESP_UberBar);
                            gui::checkbox("Aimbot FOV", CFG::Aimbot_DrawFOV);
                            gui::slider("Field of View", &CFG::Aimbot_FOV, 0.f, 180.f);
                        }
                        gui::end_group();
                    }
                    // SKINS TAB
                    if (active_tab == 3) // PLAYERS TAB
                    {
                        // Lista de jogadores (lado esquerdo)
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

                                // Determinar cor do nome baseado na flag
                                ImVec4 nameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default: branco
                                if (priority.Cheater)
                                    nameColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                                else if (priority.RetardLegit)
                                    nameColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
                                else if (priority.CheaterLight)
                                    nameColor = ImVec4(1.0f, 0.6f, 0.6f, 1.0f);
                                else if (priority.Suspect)
                                    nameColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                                else if (priority.RijinUser)
                                    nameColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
                                else if (priority.LmaoboxUser)
                                    nameColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
                                else if (priority.NethookUser)
                                    nameColor = ImVec4(0.5f, 0.0f, 0.5f, 1.0f);
                                else if (priority.Ignored)
                                    nameColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

                                // Team indicator
                                int team = pBaseEntity->m_iTeamNum();
                                const char* teamTag = team == 2 ? "[RED]" : team == 3 ? "[BLU]" : "[SPEC]";
                                ImVec4 teamColor = team == 2 ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) :
                                    team == 3 ? ImVec4(0.2f, 0.5f, 1.0f, 1.0f) :
                                    ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

                                // Renderizar linha do jogador
                                ImGui::TextColored(teamColor, "%s", teamTag);
                                ImGui::SameLine();
                                ImGui::PushStyleColor(ImGuiCol_Text, nameColor);
                                ImGui::Selectable(pi.name, false, 0, ImVec2(320.0f, 0));
                                ImGui::PopStyleColor();

                                // Menu de contexto (botão direito)
                                if (ImGui::BeginPopupContextItem())
                                {
                                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", pi.name);
                                    ImGui::Spacing();

                                    // Botões de flag em grid 2x4
                                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));

                                    // Linha 1
                                    if (ImGui::Button("Clear All", ImVec2(100, 0)))
                                    {
                                        F::Players->Mark(i, PlayerPriority{});
                                        ImGui::CloseCurrentPopup();
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button(priority.Cheater ? "✓ Cheater" : "Cheater", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.Cheater = true;
                                        F::Players->Mark(i, priority);
                                    }

                                    // Linha 2
                                    if (ImGui::Button(priority.RetardLegit ? "✓ Retard" : "Retard", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.RetardLegit = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button(priority.CheaterLight ? "✓ Light" : "Light", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.CheaterLight = true;
                                        F::Players->Mark(i, priority);
                                    }

                                    // Linha 3
                                    if (ImGui::Button(priority.Suspect ? "✓ Suspect" : "Suspect", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.Suspect = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button(priority.RijinUser ? "✓ Rijin" : "Rijin", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.RijinUser = true;
                                        F::Players->Mark(i, priority);
                                    }

                                    // Linha 4
                                    if (ImGui::Button(priority.LmaoboxUser ? "✓ Lmaobox" : "Lmaobox", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.LmaoboxUser = true;
                                        F::Players->Mark(i, priority);
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button(priority.NethookUser ? "✓ Nethook" : "Nethook", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.NethookUser = true;
                                        F::Players->Mark(i, priority);
                                    }

                                    // Linha 5
                                    if (ImGui::Button(priority.Ignored ? "✓ Ignored" : "Ignored", ImVec2(100, 0)))
                                    {
                                        priority = {};
                                        priority.Ignored = true;
                                        F::Players->Mark(i, priority);
                                    }

                                    ImGui::PopStyleVar(2);
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

                        // Legenda (lado direito)
                        ImGui::SameLine(390);
                        if (gui::begin_group_scrollable("LEGEND & INFO", ImVec2(380, 500), 5.0f, 5.0f))
                        {
                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "TEAM INDICATORS");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "[RED]");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.2f, 0.5f, 1.0f, 1.0f), "[BLU]");
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[SPEC]");
                            ImGui::Spacing();
                            ImGui::Spacing();

                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "PLAYER FLAGS");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Clean Player");
                            ImGui::TextDisabled("No flags assigned");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Cheater");
                            ImGui::TextDisabled("Confirmed cheater");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Retard Legit");
                            ImGui::TextDisabled("Suspicious legit player");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "Cheater Light");
                            ImGui::TextDisabled("Likely soft cheats");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Suspect");
                            ImGui::TextDisabled("Under observation");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Rijin User");
                            ImGui::TextDisabled("Using Rijin cheat");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Lmaobox User");
                            ImGui::TextDisabled("Using Lmaobox");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.5f, 0.0f, 0.5f, 1.0f), "Nethook User");
                            ImGui::TextDisabled("Using Nethook");
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Ignored");
                            ImGui::TextDisabled("Player is ignored");
                            ImGui::Spacing();
                            ImGui::Spacing();

                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "HOW TO USE");
                            ImGui::Spacing();
                            ImGui::TextWrapped("Right-click any player to assign flags using the button grid.");
                            ImGui::Spacing();
                            ImGui::TextWrapped("Manual flags persist between sessions and appear with colored names.");
                            ImGui::Spacing();
                            ImGui::TextWrapped("Use 'Clear All' to remove all flags from a player.");
                        }
                        gui::end_group_scrollable();
                    }
                    // MISC TAB

                    if (active_tab == 4)
                    {
                    }
                    ImGui::EndGroup();
                }
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
    App->Shutdown();
    Sleep(500);
    FreeLibraryAndExitThread(hmod, 0);
    return 0;
}
/*
DWORD WINAPI MainThread(LPVOID lpReserved)
{
    hmod = static_cast<HMODULE>(lpReserved);
    //App->Start();
    bool attached = false;
    while (!attached)
    {
        if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
        {
            kiero::bind(42, (void**)&oEndScene, hkEndScene);
            while (window == NULL) {
                window = GetProcessWindow();
                Sleep(100);
            }
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
            attached = true;
        }
        Sleep(100);
    }
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
    Sleep(200);
    FreeLibraryAndExitThread(hmod, 0);
    return 0;
}
*/
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