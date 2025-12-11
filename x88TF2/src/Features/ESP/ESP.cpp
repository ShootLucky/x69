#include "ESP.h"
#include "../src/SDK/TF2/interface.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include "CFG.h"
#include "../src/SDK/SDK.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstring> // for strcmp
#define PI 3.14159265358979323846f
#define TF_COND_STEALTHED 4
static bool IsFiniteVec(const Vec3& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
static bool InCond(C_TFPlayer* pPlayer, int cond)
{
    return (pPlayer->m_nPlayerCond() & (1 << cond)) != 0;
}
void CESP::Init()
{
    if (m_bInitialized) return;
    m_bInitialized = true;
}
void CESP::Shutdown()
{
    if (!m_bInitialized) return;
    m_bInitialized = false;
}
// Apenas desenha caixa (box ESP) — funções auxiliares simples
void CESP::DrawBox(int left, int top, int w, int h, const Color_t& clr)
{
    if (w <= 0 || h <= 0) return;
    H::Draw->OutlinedRect(left, top, w, h, clr);
}
void DrawBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    if (!H::Draw) return;
    H::Draw->Line(static_cast<int>(std::round(a.x)), static_cast<int>(std::round(a.y)),
        static_cast<int>(std::round(b.x)), static_cast<int>(std::round(b.y)),
        clr);
}
void DrawThinLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    if (!H::Draw) return;
    int ax = static_cast<int>(std::round(a.x));
    int ay = static_cast<int>(std::round(a.y));
    int bx = static_cast<int>(std::round(b.x));
    int by = static_cast<int>(std::round(b.y));
    // Draw only main line
    H::Draw->Line(ax, ay, bx, by, clr);
}
void DrawOutlinedLine(const Vec3& a, const Vec3& b, const Color_t& clr, const Color_t& outlineClr = Color_t(0, 0, 0, 255))
{
    if (!H::Draw) return;
    int ax = static_cast<int>(std::round(a.x));
    int ay = static_cast<int>(std::round(a.y));
    int bx = static_cast<int>(std::round(b.x));
    int by = static_cast<int>(std::round(b.y));
    float dx = static_cast<float>(bx - ax);
    float dy = static_cast<float>(by - ay);
    int off = 1;
    if (std::abs(dx) > std::abs(dy)) {
        // Horizontal-ish line, offset vertically
        H::Draw->Line(ax, ay - off, bx, by - off, outlineClr);
        H::Draw->Line(ax, ay + off, bx, by + off, outlineClr);
    }
    else {
        // Vertical-ish line, offset horizontally
        H::Draw->Line(ax - off, ay, bx - off, by, outlineClr);
        H::Draw->Line(ax + off, ay, bx + off, by, outlineClr);
    }
    // Draw main line
    H::Draw->Line(ax, ay, bx, by, clr);
}
void DrawSmoothBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr, bool useAA = true)
{
    if (!H::Draw) return;
    int ax = static_cast<int>(std::round(a.x));
    int ay = static_cast<int>(std::round(a.y));
    int bx = static_cast<int>(std::round(b.x));
    int by = static_cast<int>(std::round(b.y));
    // Draw main line
    H::Draw->Line(ax, ay, bx, by, clr);
    if (useAA) {
        // Draw offset lines for anti-aliasing effect with lower alpha to make thinner appearance
        Color_t aaClr = Color_t(clr.r, clr.g, clr.b, 50); // Reduced alpha for weaker AA
        float dx = static_cast<float>(bx - ax);
        float dy = static_cast<float>(by - ay);
        int off = 1;
        if (std::abs(dx) > std::abs(dy)) {
            // Horizontal-ish line, offset vertically
            H::Draw->Line(ax, ay - off, bx, by - off, aaClr);
            H::Draw->Line(ax, ay + off, bx, by + off, aaClr);
        }
        else {
            // Vertical-ish line, offset horizontally
            H::Draw->Line(ax - off, ay, bx - off, by, aaClr);
            H::Draw->Line(ax + off, ay, bx + off, by, aaClr);
        }
    }
}
void DrawScreenLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    DrawSmoothBoneLine(a, b, clr);
}
static void DrawImpactBox(const Vec3& worldPos, const Color_t& clr) {
    if (!H::Draw) return;
    Vec3 mins(-4.0f, -4.0f, -4.0f);
    Vec3 maxs(4.0f, 4.0f, 4.0f);
    Vec3 points[8] = {
        { worldPos.x + mins.x, worldPos.y + mins.y, worldPos.z + mins.z },
        { worldPos.x + mins.x, worldPos.y + maxs.y, worldPos.z + mins.z },
        { worldPos.x + maxs.x, worldPos.y + maxs.y, worldPos.z + mins.z },
        { worldPos.x + maxs.x, worldPos.y + mins.y, worldPos.z + mins.z },
        { worldPos.x + maxs.x, worldPos.y + maxs.y, worldPos.z + maxs.z },
        { worldPos.x + maxs.x, worldPos.y + mins.y, worldPos.z + maxs.z },
        { worldPos.x + mins.x, worldPos.y + maxs.y, worldPos.z + maxs.z },
        { worldPos.x + mins.x, worldPos.y + mins.y, worldPos.z + maxs.z }
    };
    Vec3 scr[8];
    bool projOk = true;
    for (int i = 0; i < 8; ++i) {
        if (!H::Draw->W2S(points[i], scr[i])) {
            projOk = false;
            break;
        }
    }
    if (!projOk) return;
    // Removido: 2D fill e outline para focar apenas no wireframe 3D com outline
    // Color_t fillClr(clr.r, clr.g, clr.b, 100);
    // H::Draw->Rect(static_cast<int>(std::round(left)), static_cast<int>(std::round(top)), w, h, fillClr);
    // H::Draw->OutlinedRect(static_cast<int>(std::round(left)), static_cast<int>(std::round(top)), w, h, clr);
    // Draw 3D wireframe
    const std::pair<int, int> edges[] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for (auto& e : edges) {
        DrawOutlinedLine(scr[e.first], scr[e.second], clr);
    }
}
// --- Helper: desenha apenas o wireframe projetado (sem pontos/glow) ---
static void DrawProjectedHitboxWire(const Vec3 proj[8], const Color_t& clr, bool useAA = true)
{
    if (!H::Draw) return;
    for (int i = 0; i < 8; ++i)
    {
        if (!std::isfinite(proj[i].x) || !std::isfinite(proj[i].y))
            return;
    }
    const std::pair<int, int> edges[] = {
        {0,1},{1,2},{2,3},{3,0},
        {7,6},{6,4},{4,5},{5,7},
        {0,7},{1,6},{2,4},{3,5}
    };
    for (auto& e : edges)
    {
        const Vec3& A = proj[e.first];
        const Vec3& B = proj[e.second];
        // Use DrawSmoothBoneLine with useAA
        DrawSmoothBoneLine(A, B, clr, useAA);
    }
}
static void DrawOffscreenArrow(const Vec3& origin, const Color_t& clr)
{
    auto pLocal = H::Entities->GetLocal();
    if (!pLocal) return;
    Vec3 screen;
    if (H::Draw->W2S(origin, screen))
    {
        int screenW, screenH;
        I::EngineClient->GetScreenSize(screenW, screenH);
        if (screen.x > 0 && screen.x < screenW && screen.y > 0 && screen.y < screenH)
            return; // on screen, no need
    }
    int screenW, screenH;
    I::EngineClient->GetScreenSize(screenW, screenH);
    Vec3 localEye = pLocal->GetShootPos();
    Vec3 dir = origin - localEye;
    dir.Normalize();
    Vec3 viewAngles;
    I::EngineClient->GetViewAngles(viewAngles);
    float angle = atan2(dir.y, dir.x) - DEG2RAD(viewAngles.y); // fixed
    float radius = 80.0f; // Reduced radius to be closer to center (smaller arrow near center)
    float centerX = screenW / 2.0f;
    float centerY = screenH / 2.0f;
    Vec3 arrowTip(centerX + radius * cos(angle), centerY + radius * sin(angle), 0);
    float sideAngle = PI / 6; // 30 deg
    Vec3 side1(centerX + (radius - 10) * cos(angle + sideAngle), centerY + (radius - 10) * sin(angle + sideAngle), 0); // Smaller sides
    Vec3 side2(centerX + (radius - 10) * cos(angle - sideAngle), centerY + (radius - 10) * sin(angle - sideAngle), 0);
    DrawSmoothBoneLine(arrowTip, side1, clr);
    DrawSmoothBoneLine(arrowTip, side2, clr);
    DrawSmoothBoneLine(side1, side2, clr);
}
static void DrawFOVCircle(float fov, const Color_t& color) {
    if (fov <= 0.0f) return;
    int w, h;
    I::EngineClient->GetScreenSize(w, h);
    float centerX = w / 2.0f;
    float centerY = h / 2.0f;
    float viewFOV = 90.0f;
    ConVar* fov_desired = I::CVar->FindVar("fov_desired");
    if (fov_desired) viewFOV = fov_desired->GetFloat();
    if (viewFOV <= 0.0f) viewFOV = 90.0f;
    float radius = tanf(DEG2RAD(fov) / 2.0f) / tanf(DEG2RAD(viewFOV) / 2.0f) * (h / 2.0f);
    H::Draw->OutlinedCircle(centerX, centerY, radius, 64, color);
}
// ---------------------------------------------------------------
void CESP::Run()
{
    if (!CFG::ESP_Enable) return;
    if (!m_bInitialized) Init();
    H::Draw->UpdateScreenSize();
    H::Draw->UpdateW2SMatrix();
    auto pLocal = H::Entities->GetLocal();
    if (!pLocal) return;
    // Bullet Tracers drawing
    const int maxClients = I::EngineClient->GetMaxClients();
    const int highestIndex = I::ClientEntityList->GetHighestEntityIndex();
    const int end = std::max(maxClients, highestIndex);
    int localTeam = -1;
    pLocal->IsInValidTeam(&localTeam);
    int localIndex = I::EngineClient->GetLocalPlayer();
    auto pResource = GetTFPlayerResource();
    // Opcional: Atualiza cache para objetivos (como Intel) - baseado em Amalgam
    H::Entities->UpdateCache();
    for (int i = 0; i <= end; ++i)
    {
        IClientEntity* pClientEnt = I::ClientEntityList->GetClientEntity(i);
        if (!pClientEnt) continue;
        C_BaseEntity* pBase = pClientEnt->As<C_BaseEntity>();
        if (!pBase) continue;
        if (pClientEnt->IsDormant()) continue;
        const char* networkName = pBase->GetClientClass()->m_pNetworkName;
        int team = pBase->m_iTeamNum();
        bool isTeammate = (localTeam != -1 && team == localTeam);
        bool isEnemy = !isTeammate && team > 1; // >1 to exclude spec
        bool drawESP = false;
        bool drawSkeleton = false;
        bool drawChamsBox = false;
        bool isPlayer = false;
        bool isBuilding = false;
        bool isPickup = false;
        bool isFlag = false;
        std::string name = "";
        int health = 0;
        int max_health = 100;
        Color_t clr = Color_t(255, 255, 255, 255);
        if (strcmp(networkName, "CTFPlayer") == 0)
        {
            C_TFPlayer* pPlayer = static_cast<C_TFPlayer*>(pBase);
            if (!pPlayer) continue;
            if (pPlayer->m_lifeState() != LIFE_ALIVE) continue;
            isPlayer = true;
            bool isLocal = (i == localIndex);
            if (isLocal && !CFG::ESP_LocalPlayer && !CFG::ESP_ChamsLocalPlayer && !CFG::ESP_SkeletonLocalPlayer) continue;
            bool isCloaked = InCond(pPlayer, TF_COND_STEALTHED);
            if (isCloaked && (CFG::ESP_HideCloaked || CFG::ESP_ChamsHideCloaked || CFG::ESP_SkeletonHideCloaked)) {
                if ((CFG::ESP_HideCloaked && CFG::ESP_ChamsHideCloaked && CFG::ESP_SkeletonHideCloaked) || (!CFG::ESP_ChamsLocalPlayer && !CFG::ESP_SkeletonLocalPlayer)) continue;
            }
            clr = isLocal ? Color_t(255, 255, 255, 255) : (team == TF_TEAM_RED ? Color_t(255, 0, 0, 255) : Color_t(0, 0, 255, 255));
            drawESP = (isLocal ? CFG::ESP_LocalPlayer : true) && !(CFG::ESP_Team && isTeammate) && !(isCloaked && CFG::ESP_HideCloaked);
            drawSkeleton = (isLocal ? CFG::ESP_SkeletonLocalPlayer : CFG::ESP_Skeleton) && !(CFG::ESP_SkeletonTeam && isTeammate) && !(isCloaked && CFG::ESP_SkeletonHideCloaked);
            drawChamsBox = (isLocal ? CFG::ESP_ChamsLocalPlayer : CFG::ESP_ChamsBox) && !(CFG::ESP_ChamsTeam && isTeammate) && !(isCloaked && CFG::ESP_ChamsHideCloaked);
            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(i, &info)) name = info.name ? info.name : "unknown";
            if (pResource) {
                static int healthOffset = NetVars::GetNetVar("CPlayerResource", "m_iHealth");
                static int maxHealthOffset = NetVars::GetNetVar("CTFPlayerResource", "m_iMaxHealth");
                health = *reinterpret_cast<int*>((uintptr_t)pResource + healthOffset + i * sizeof(int));
                max_health = *reinterpret_cast<int*>((uintptr_t)pResource + maxHealthOffset + i * sizeof(int));
            }
            if (max_health <= 0) max_health = pPlayer->GetMaxHealth();
            if (max_health <= 0) max_health = 100;
            health = std::clamp(health, 0, max_health);
        }
        else if (strcmp(networkName, "CObjectSentrygun") == 0 || strcmp(networkName, "CObjectDispenser") == 0 || strcmp(networkName, "CObjectTeleporter") == 0)
        {
            if (!CFG::ESP_Build && !CFG::ESP_ChamsBuild && !CFG::ESP_SkeletonBuild) continue;
            isBuilding = true;
            if (CFG::ESP_BuildOnlyEnemy && !isEnemy) continue;
            clr = (team == TF_TEAM_RED ? Color_t(255, 0, 0, 255) : Color_t(0, 0, 255, 255));
            if (strcmp(networkName, "CObjectSentrygun") == 0) name = "Sentry";
            else if (strcmp(networkName, "CObjectDispenser") == 0) name = "Dispenser";
            else name = "Teleporter";
            int level = *reinterpret_cast<int*>((uintptr_t)pBase + NetVars::GetNetVar("CBaseObject", "m_iUpgradeLevel"));
            name += " Lvl " + std::to_string(level);
            health = static_cast<C_BaseObject*>(pBase)->m_iHealth();
            max_health = static_cast<C_BaseObject*>(pBase)->m_iMaxHealth();
            drawESP = CFG::ESP_Build && !(CFG::ESP_BuildOnlyEnemy && !isEnemy);
            drawSkeleton = CFG::ESP_SkeletonBuild && !(CFG::ESP_SkeletonBuildOnlyEnemy && !isEnemy);
            drawChamsBox = CFG::ESP_ChamsBuild && !(CFG::ESP_ChamsBuildOnlyEnemy && !isEnemy);
        }
        else if ((strcmp(networkName, "CTFAmmoPack") == 0 ||
            strstr(networkName, "item_healthkit_") ||
            strstr(networkName, "item_ammopack_")) ||
            (strcmp(networkName, "CBaseAnimating") == 0 &&
                (H::Entities->IsHealthPack(pBase) || H::Entities->IsAmmoPack(pBase))))
        {
            // Skip se for debris de building (ex: sentry broken -> debris model similar a ammo, mas class != CTFAmmoPack e sem strstr)
            if (strstr(networkName, "debris") && !strstr(networkName, "item_")) continue;
            if (!CFG::ESP_Pickups && !CFG::ESP_ChamsPickups && !CFG::ESP_SkeletonPickups) continue; // Assumindo CFGs para chams/skeleton em pickups
            isPickup = true;
            bool isAmmo = (strcmp(networkName, "CTFAmmoPack") == 0 ||
                strstr(networkName, "ammopack") ||
                H::Entities->IsAmmoPack(pBase));
            if (isAmmo) {
                name = "Ammo";
                clr = Color_t(255, 215, 0, 255); // gold
            }
            else {
                name = "Medkit";
                clr = Color_t(0, 255, 0, 255); // green
            }
            drawESP = CFG::ESP_Pickups;
            drawSkeleton = CFG::ESP_SkeletonPickups; // Nova CFG assumida
            drawChamsBox = CFG::ESP_ChamsPickups; // Nova CFG assumida
        }
        else if (strcmp(networkName, "CCaptureFlag") == 0 ||
            strcmp(networkName, "CTFItemTeamFlag") == 0 ||
            strcmp(networkName, "CItemTeamFlag") == 0 ||
            strcmp(networkName, "item_teamflag") == 0 ||
            strcmp(networkName, "CTeamControlPoint") == 0 ||
            strcmp(networkName, "CFuncTrackTrain") == 0)
        {
            if (CFG::ESP_Team && isTeammate) continue; // Respeita opção de team: skip se for do time local
            isFlag = true;
            if (strstr(networkName, "flag") || strstr(networkName, "Flag")) name = "Intel";
            else if (strcmp(networkName, "CTeamControlPoint") == 0) name = "Control Point";
            else name = "Payload Cart";
            drawESP = CFG::ESP_CaptureFlag;
            // Chams e Skeleton independentes de drawESP
            drawChamsBox = CFG::ESP_ChamsCaptureFlag;
            drawSkeleton = CFG::ESP_SkeletonCaptureFlag;
            clr = Color_t(255, 255, 0, 255); // yellow
        }
        else continue;
        Vec3 origin = pBase->GetAbsOrigin();
        Vec3 mins = pBase->m_vecMins();
        Vec3 maxs = pBase->m_vecMaxs();
        if (!IsFiniteVec(mins) || !IsFiniteVec(maxs) || !IsFiniteVec(origin)) continue;
        // Melhoria para box dinâmico: Use hull_min/max do modelo se mins/maxs da entity forem zero/inválidos
        if (mins.Length() < 0.1f || maxs.Length() < 0.1f) { // Threshold para detectar bbox inválido
            if (auto pAnim = pBase->As<C_BaseAnimating>()) {
                auto pModel = pAnim->GetModel();
                if (pModel) {
                    auto pStudio = I::ModelInfoClient->GetStudiomodel(pModel);
                    if (pStudio) {
                        mins = pStudio->hull_min;
                        maxs = pStudio->hull_max;
                        // Fallback hardcoded se hull ainda inválido (raro)
                        if (mins.Length() < 0.1f || maxs.Length() < 0.1f) {
                            if (isPickup) {
                                mins = Vec3(-8, -8, 0);
                                maxs = Vec3(8, 8, 16);
                            }
                            else if (isFlag) {
                                mins = Vec3(-12, -6, 0);
                                maxs = Vec3(12, 6, 30);
                            }
                            else if (isPlayer) {
                                mins = Vec3(-24, -24, 0);
                                maxs = Vec3(24, 24, 82);
                            }
                        }
                    }
                }
            }
        }
        // Removido: Atualiza cache para objetivos (como Intel) - baseado em Amalgam
        mins += origin;
        maxs += origin;
        Vec3 points[8] = {
            { mins.x, mins.y, mins.z },
            { mins.x, maxs.y, mins.z },
            { maxs.x, maxs.y, mins.z },
            { maxs.x, mins.y, mins.z },
            { maxs.x, maxs.y, maxs.z },
            { maxs.x, mins.y, maxs.z },
            { mins.x, maxs.y, maxs.z },
            { mins.x, mins.y, maxs.z }
        };
        Vec3 screenPts[8];
        bool anyFailed = false;
        for (int k = 0; k < 8; ++k)
        {
            if (!H::Draw->W2S(points[k], screenPts[k]))
            {
                anyFailed = true;
                break;
            }
        }
        if (anyFailed && !CFG::ESP_Offscreen) continue;
        float left = screenPts[0].x, top = screenPts[0].y, right = screenPts[0].x, bottom = screenPts[0].y;
        for (int k = 1; k < 8; ++k)
        {
            if (screenPts[k].x < left) left = screenPts[k].x;
            if (screenPts[k].x > right) right = screenPts[k].x;
            if (screenPts[k].y < top) top = screenPts[k].y;
            if (screenPts[k].y > bottom) bottom = screenPts[k].y;
        }
        if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) || !std::isfinite(bottom)) continue;
        int width = static_cast<int>(std::round(right - left));
        int height = static_cast<int>(std::round(bottom - top));
        if (width < 2 || height < 2) continue;
        if (drawESP || drawChamsBox || drawSkeleton) { // Desenhar mesmo se só chams/skeleton
            bool draw_box = CFG::ESP_Box;
            bool draw_name = CFG::ESP_Name;
            if (isPickup) {
                draw_box = CFG::ESP_PickupsBox;
                draw_name = CFG::ESP_PickupsName;
            }
            if (isFlag) {
                draw_box = CFG::ESP_BoxCapture;
                draw_name = CFG::ESP_NameCapture;
            }
            if (draw_box && drawESP) // Box só se drawESP
                DrawBox(static_cast<int>(std::round(left)), static_cast<int>(std::round(top)), width, height, clr);
            if (draw_name && !name.empty())
            {
                const CFont& font = H::Fonts->Get(EFonts::ESP);
                H::Draw->String(font, static_cast<int>((left + right) / 2.0f), static_cast<int>(top) - 15, Color_t(255, 255, 255, 255), POS_CENTERX, name.c_str());
            }
            if (CFG::ESP_Health && max_health > 0 && (isPlayer || isBuilding))
            {
                health = std::clamp(health, 0, max_health);
                int barX = static_cast<int>(std::round(left)) - 6;
                int barY = static_cast<int>(std::round(top)) - 1;
                int barW = 4;
                int barH = height + 2;
                H::Draw->Rect(barX, barY, barW, barH, Color_t(0, 0, 0, 200));
                if (barH > 2 && health > 0)
                {
                    float ratio = static_cast<float>(health) / static_cast<float>(max_health);
                    int fillH = static_cast<int>(std::round((barH - 2) * ratio));
                    int fillY = barY + (barH - 1) - fillH;
                    int red = static_cast<int>(255 * (1.0f - ratio));
                    int green = static_cast<int>(255 * ratio);
                    Color_t c(static_cast<unsigned char>(red), static_cast<unsigned char>(green), 0, 255);
                    H::Draw->Rect(barX + 1, fillY, 2, fillH, c);
                }
            }
        }
        if (CFG::ESP_Offscreen && isEnemy && anyFailed && isPlayer) {
            DrawOffscreenArrow(origin, clr);
        }
        if ((drawSkeleton || drawChamsBox) && (isPlayer || isBuilding || isFlag || isPickup)) // Adicionado isPickup
        {
            matrix3x4_t boneMatrix[128] = {};
            const int maxBones = 128;
            bool ok = pClientEnt->SetupBones(boneMatrix, maxBones, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime);
            if (!ok) continue;
            auto pAnimating = pBase->As<C_BaseAnimating>();
            if (!pAnimating) continue;
            auto pModel = pAnimating->GetModel();
            if (!pModel) continue;
            auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
            if (!pHDR) continue;
            int numBones = std::min(pHDR->numbones, maxBones);
            int hitboxSet = pAnimating->m_nHitboxSet();
            int hitboxCount = pHDR->iHitboxCount(hitboxSet);
            if (drawSkeleton)
            {
                std::map<std::string, Vec3> keyBoneScreens;
                std::map<std::string, int> keyBoneIndices = {
                    {"bip_head", -1},
                    {"bip_neck", -1},
                    {"bip_spine_3", -1},
                    {"bip_spine_2", -1},
                    {"bip_spine_1", -1},
                    {"bip_spine_0", -1},
                    {"bip_pelvis", -1},
                    {"bip_hip_L", -1},
                    {"bip_knee_L", -1},
                    {"bip_foot_L", -1},
                    {"bip_hip_R", -1},
                    {"bip_knee_R", -1},
                    {"bip_foot_R", -1},
                    {"bip_upperArm_L", -1},
                    {"bip_lowerArm_L", -1},
                    {"bip_hand_L", -1},
                    {"bip_upperArm_R", -1},
                    {"bip_lowerArm_R", -1},
                    {"bip_hand_R", -1}
                };
                for (int b = 0; b < numBones; ++b)
                {
                    mstudiobone_t* pBone = pHDR->pBone(b);
                    if (!pBone) continue;
                    std::string boneName = pBone->pszName();
                    auto it = keyBoneIndices.find(boneName);
                    if (it != keyBoneIndices.end())
                        it->second = b;
                }
                for (const auto& kv : keyBoneIndices)
                {
                    int b = kv.second;
                    if (b == -1) continue;
                    Vec3 bonePosWorld{};
                    Math::VectorTransform(Vec3{ 0.0f, 0.0f, 0.0f }, boneMatrix[b], bonePosWorld);
                    Vec3 screenPos;
                    if (H::Draw->W2S(bonePosWorld, screenPos))
                    {
                        keyBoneScreens[kv.first] = screenPos;
                    }
                }
                Color_t boneColor = clr;
                const std::vector<std::vector<std::string>> chains = {
                    {"bip_head", "bip_neck", "bip_spine_3", "bip_spine_2", "bip_spine_1", "bip_spine_0", "bip_pelvis"},
                    {"bip_pelvis", "bip_hip_L", "bip_knee_L", "bip_foot_L"},
                    {"bip_pelvis", "bip_hip_R", "bip_knee_R", "bip_foot_R"},
                    {"bip_spine_3", "bip_upperArm_L", "bip_lowerArm_L", "bip_hand_L"},
                    {"bip_spine_3", "bip_upperArm_R", "bip_lowerArm_R", "bip_hand_R"}
                };
                for (const auto& chain : chains)
                {
                    for (size_t idx = 0; idx < chain.size() - 1; ++idx)
                    {
                        auto it1 = keyBoneScreens.find(chain[idx]);
                        auto it2 = keyBoneScreens.find(chain[idx + 1]);
                        if (it1 != keyBoneScreens.end() && it2 != keyBoneScreens.end())
                        {
                            DrawSmoothBoneLine(it1->second, it2->second, boneColor); // Use smooth lines with adjusted alpha
                        }
                    }
                }
                // Draw circle with cat ears on head
                auto headIt = keyBoneScreens.find("bip_head");
                if (headIt != keyBoneScreens.end())
                {
                    // Compute r based on head hitbox size
                    float r = 10.0f; // default
                    auto pHeadBox = pHDR->pHitbox(HITBOX_HEAD, hitboxSet);
                    if (pHeadBox)
                    {
                        int headBoneIdx = pHeadBox->bone;
                        if (headBoneIdx >= 0 && headBoneIdx < numBones)
                        {
                            Vec3 headCorners[8];
                            Vec3 modelHeadCorners[8] = {
                                { pHeadBox->bbmin.x, pHeadBox->bbmin.y, pHeadBox->bbmin.z },
                                { pHeadBox->bbmin.x, pHeadBox->bbmax.y, pHeadBox->bbmin.z },
                                { pHeadBox->bbmax.x, pHeadBox->bbmax.y, pHeadBox->bbmin.z },
                                { pHeadBox->bbmax.x, pHeadBox->bbmin.y, pHeadBox->bbmin.z },
                                { pHeadBox->bbmax.x, pHeadBox->bbmax.y, pHeadBox->bbmax.z },
                                { pHeadBox->bbmax.x, pHeadBox->bbmin.y, pHeadBox->bbmax.z },
                                { pHeadBox->bbmin.x, pHeadBox->bbmax.y, pHeadBox->bbmax.z },
                                { pHeadBox->bbmin.x, pHeadBox->bbmin.y, pHeadBox->bbmax.z }
                            };
                            for (int c = 0; c < 8; ++c) Math::VectorTransform(modelHeadCorners[c], boneMatrix[headBoneIdx], headCorners[c]);
                            Vec3 headScr[8];
                            bool headProjOk = true;
                            for (int c = 0; c < 8; ++c)
                            {
                                if (!H::Draw->W2S(headCorners[c], headScr[c]))
                                {
                                    headProjOk = false;
                                    break;
                                }
                            }
                            if (headProjOk)
                            {
                                float headLeft = headScr[0].x, headRight = headScr[0].x, headTop = headScr[0].y, headBottom = headScr[0].y;
                                for (int c = 1; c < 8; ++c)
                                {
                                    if (headScr[c].x < headLeft) headLeft = headScr[c].x;
                                    if (headScr[c].x > headRight) headRight = headScr[c].x;
                                    if (headScr[c].y < headTop) headTop = headScr[c].y;
                                    if (headScr[c].y > headBottom) headBottom = headScr[c].y;
                                }
                                float headWidth = headRight - headLeft;
                                float headHeight = headBottom - headTop;
                                r = std::min(headWidth, headHeight) / 2.0f; // Use min to make more proportional, fitting the smaller dimension
                            }
                        }
                    }
                    Vec3 screenHead = headIt->second;
                    Color_t circleClr = boneColor;
                    int segments = 64; // Increased segments for smoother circle
                    Vec3 lastP;
                    bool first = true;
                    for (int s = 0; s <= segments; s++)
                    {
                        float theta = 2.0f * PI * static_cast<float>(s) / static_cast<float>(segments);
                        float px = screenHead.x + r * std::cos(theta);
                        float py = screenHead.y + r * std::sin(theta);
                        Vec3 p(px, py, 0);
                        if (!first)
                            DrawSmoothBoneLine(lastP, p, circleClr); // Use smooth lines with adjusted alpha
                        lastP = p;
                        first = false;
                    }
                    // Cat ears on top
                    float earH = r * 0.6f; // Reduced ear height for better proportion
                    float earBaseAngle = PI / 6.0f; // 30 degrees from top for spacing
                    // Left ear base left and right on circle
                    float leftBaseLTheta = 3 * PI / 2 - earBaseAngle;
                    float leftBaseRTheta = 3 * PI / 2 - earBaseAngle / 2;
                    Vec3 leftEarBaseL(screenHead.x + r * std::cos(leftBaseLTheta), screenHead.y + r * std::sin(leftBaseLTheta), 0);
                    Vec3 leftEarBaseR(screenHead.x + r * std::cos(leftBaseRTheta), screenHead.y + r * std::sin(leftBaseRTheta), 0);
                    Vec3 leftEarTip(screenHead.x + r * std::cos(3 * PI / 2 - earBaseAngle * 0.75f), screenHead.y + r * std::sin(3 * PI / 2 - earBaseAngle * 0.75f) - earH, 0);
                    DrawSmoothBoneLine(leftEarBaseL, leftEarTip, circleClr); // Use smooth lines with adjusted alpha
                    DrawSmoothBoneLine(leftEarTip, leftEarBaseR, circleClr); // Use smooth lines with adjusted alpha
                    // Right ear symmetric
                    float rightBaseLTheta = 3 * PI / 2 + earBaseAngle / 2;
                    float rightBaseRTheta = 3 * PI / 2 + earBaseAngle;
                    Vec3 rightEarBaseL(screenHead.x + r * std::cos(rightBaseLTheta), screenHead.y + r * std::sin(rightBaseLTheta), 0);
                    Vec3 rightEarBaseR(screenHead.x + r * std::cos(rightBaseRTheta), screenHead.y + r * std::sin(rightBaseRTheta), 0);
                    Vec3 rightEarTip(screenHead.x + r * std::cos(3 * PI / 2 + earBaseAngle * 0.75f), screenHead.y + r * std::sin(3 * PI / 2 + earBaseAngle * 0.75f) - earH, 0);
                    DrawSmoothBoneLine(rightEarBaseL, rightEarTip, circleClr); // Use smooth lines with adjusted alpha
                    DrawSmoothBoneLine(rightEarTip, rightEarBaseR, circleClr); // Use smooth lines with adjusted alpha
                }
            }
            if (drawChamsBox)
            {
                // Estilo moderno: Adiciona pulso animado para brilho (sem outline branco/glow externo)
                float pulse = (std::sin(I::GlobalVars->curtime * 4.0f) + 1.0f) * 0.5f * 0.3f + 0.7f; // Pulso suave entre 0.7 e 1.0 para brilho moderno
                Color_t wireColor = Color_t(
                    static_cast<unsigned char>(std::min(255.0f, clr.r * pulse)),
                    static_cast<unsigned char>(std::min(255.0f, clr.g * pulse)),
                    static_cast<unsigned char>(std::min(255.0f, clr.b * pulse)),
                    clr.a
                );
                float dist = (pLocal->GetShootPos() - origin).Length();
                bool useAA = dist < 1500.0f; // Use AA only when close
                for (int hb = 0; hb < hitboxCount; ++hb)
                {
                    auto pBox = pHDR->pHitbox(hb, hitboxSet);
                    if (!pBox) continue;
                    int boneIdx = pBox->bone;
                    if (boneIdx < 0 || boneIdx >= numBones) continue;
                    Vec3 modelCorners[8] = {
                        { pBox->bbmin.x, pBox->bbmin.y, pBox->bbmin.z },
                        { pBox->bbmin.x, pBox->bbmax.y, pBox->bbmin.z },
                        { pBox->bbmax.x, pBox->bbmax.y, pBox->bbmin.z },
                        { pBox->bbmax.x, pBox->bbmin.y, pBox->bbmin.z },
                        { pBox->bbmax.x, pBox->bbmax.y, pBox->bbmax.z },
                        { pBox->bbmax.x, pBox->bbmin.y, pBox->bbmax.z },
                        { pBox->bbmin.x, pBox->bbmax.y, pBox->bbmax.z },
                        { pBox->bbmin.x, pBox->bbmin.y, pBox->bbmax.z }
                    };
                    Vec3 worldCorners[8];
                    for (int c = 0; c < 8; ++c)
                        Math::VectorTransform(modelCorners[c], boneMatrix[boneIdx], worldCorners[c]);
                    bool badWorld = false;
                    for (int c = 0; c < 8; ++c)
                    {
                        if (!IsFiniteVec(worldCorners[c])) { badWorld = true; break; }
                    }
                    if (badWorld) continue;
                    Vec3 scr[8];
                    bool fail = false;
                    for (int c = 0; c < 8; ++c)
                    {
                        if (!H::Draw->W2S(worldCorners[c], scr[c]))
                        {
                            fail = true;
                            break;
                        }
                    }
                    if (fail) continue;
                    DrawProjectedHitboxWire(scr, wireColor, useAA);
                }
            }
        }
    }
    if (CFG::Aimbot_DrawFOV) {
        DrawFOVCircle(CFG::Aimbot_FOV, Color_t(0, 0, 255, 255)); // blue for aimbot
        DrawFOVCircle(CFG::Aimbot_Projectile_FOV, Color_t(0, 255, 0, 255)); // green for project
        DrawFOVCircle(CFG::Aimbot_Melee_FOV, Color_t(255, 0, 0, 255)); // red for melee
    }
    if (CFG::Visuals_Draw_Movement_Path_Style != 0 && CFG::Aimbot_Projectile_Enable) {
        if (G::nTargetIndex > 0) {
            C_TFPlayer* pTarget = reinterpret_cast<C_TFPlayer*>(I::ClientEntityList->GetClientEntity(G::nTargetIndex));
            if (pTarget && pTarget->m_lifeState() == LIFE_ALIVE && pTarget->m_iTeamNum() != pLocal->m_iTeamNum()) {
                Vec3 pos = pTarget->GetAbsOrigin();
                Vec3 vel = pTarget->m_vecVelocity();
                float dt = I::GlobalVars->interval_per_tick;
                float gravity = SDKUtils::GetGravity() * dt;
                Color_t clr = Color_t(255, 255, 0, 255); // yellow
                int style = CFG::Visuals_Draw_Movement_Path_Style;
                int ticks = CFG::Aimbot_Projectile_TicksPredict;
                Vec3 lastPos = pos;
                Vec3 lastScr;
                H::Draw->W2S(lastPos, lastScr);
                for (int t = 1; t <= ticks; t++) {
                    pos += vel * dt;
                    if (!(pTarget->m_fFlags() & FL_ONGROUND)) {
                        vel.z -= gravity;
                    }
                    Vec3 scr;
                    if (H::Draw->W2S(pos, scr)) {
                        if (style == 1) { // line
                            DrawSmoothBoneLine(lastScr, scr, clr);
                        }
                        else if (style == 2) { // dotted
                            H::Draw->FilledCircle(scr.x, scr.y, 2, 8, clr);
                        }
                    }
                    lastPos = pos;
                    lastScr = scr;
                }
            }
        }
    }
}
CESP gESP;
