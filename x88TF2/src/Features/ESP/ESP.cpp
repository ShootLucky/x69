// esp.cpp
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
#include <limits> // for numeric_limits
#define PI 3.14159265358979323846f

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
void CESP::DrawBox2D(int left, int top, int w, int h, const Color_t& clr)
{
    DrawBox(left, top, w, h, clr);
}
void CESP::DrawBox3D(Vec3 scr[8], const Color_t& clr, bool useAA)
{
    DrawProjectedHitboxWire(scr, clr, useAA);
}
void CESP::DrawBoxCorner(int left, int top, int w, int h, const Color_t& clr)
{
    int cornerLen = std::min(w, h) / 5;
    if (cornerLen < 1) return;
    // Top-left
    H::Draw->Line(left, top, left + cornerLen, top, clr);
    H::Draw->Line(left, top, left, top + cornerLen, clr);
    // Top-right
    H::Draw->Line(left + w - cornerLen, top, left + w, top, clr);
    H::Draw->Line(left + w, top, left + w, top + cornerLen, clr);
    // Bottom-left
    H::Draw->Line(left, top + h - cornerLen, left, top + h, clr);
    H::Draw->Line(left, top + h, left + cornerLen, top + h, clr);
    // Bottom-right
    H::Draw->Line(left + w - cornerLen, top + h, left + w, top + h, clr);
    H::Draw->Line(left + w, top + h - cornerLen, left + w, top + h, clr);
}
void CESP::DrawBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    if (!H::Draw) return;
    H::Draw->Line(static_cast<int>(std::round(a.x)), static_cast<int>(std::round(a.y)),
        static_cast<int>(std::round(b.x)), static_cast<int>(std::round(b.y)),
        clr);
}
void CESP::DrawThinLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    if (!H::Draw) return;
    int ax = static_cast<int>(std::round(a.x));
    int ay = static_cast<int>(std::round(a.y));
    int bx = static_cast<int>(std::round(b.x));
    int by = static_cast<int>(std::round(b.y));
    // Draw only main line
    H::Draw->Line(ax, ay, bx, by, clr);
}
void CESP::DrawOutlinedLine(const Vec3& a, const Vec3& b, const Color_t& clr, const Color_t& outlineClr)
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
void CESP::DrawSmoothBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr, bool useAA)
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
void CESP::DrawScreenLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    DrawSmoothBoneLine(a, b, clr);
}
void CESP::DrawImpactBox(const Vec3& worldPos, const Color_t& clr) {
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
void CESP::DrawProjectedHitboxWire(const Vec3 proj[8], const Color_t& clr, bool useAA)
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
void CESP::DrawOffscreenArrow(const Vec3& origin, const Color_t& clr)
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
void CESP::DrawFOVCircle(float fov, const Color_t& color) {
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
void CESP::CustomFOV(CViewSetup* pSetup)
{
    if (!pSetup) return;
    auto pLocal = H::Entities->GetLocal();
    if (!pLocal) return;
    float fov = pSetup->fov; // default to current
    if (CFG::Visuals_CustomFov_Enable) fov = CFG::Visuals_CustomFov_Amount;
    if (InCond(pLocal, 1)) {
        if (CFG::Visuals_RemoveScopedZoom) fov = 90.0f; // or CFG::Visuals_CustomFov_Amount
    }
    pSetup->fov = fov;
}
// Helper function for GetHitboxPosition
Vec3 GetHitboxPosition(C_TFPlayer* pPlayer, int iHitbox) {
    matrix3x4_t boneMatrix[128];
    if (!pPlayer->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime)) {
        return Vec3();
    }
    auto pModel = pPlayer->GetModel();
    if (!pModel) {
        return Vec3();
    }
    auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR) {
        return Vec3();
    }
    auto pSet = pHDR->pHitboxSet(0); // Assuming hitbox set 0
    if (!pSet) {
        return Vec3();
    }
    auto pBox = pSet->pHitbox(iHitbox);
    if (!pBox) {
        return Vec3();
    }
    Vec3 vMin, vMax;
    Math::VectorTransform(pBox->bbmin, boneMatrix[pBox->bone], vMin);
    Math::VectorTransform(pBox->bbmax, boneMatrix[pBox->bone], vMax);
    return (vMin + vMax) * 0.5f;
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
    if (InCond(pLocal, 1)) {
        if (CFG::Visuals_RemoveScoped) {
            pLocal->RemoveCond(1);
        }
    }
    // For Remove Fire, assuming remove muzzle flash or similar; implementation may require additional hooks
    // Placeholder: if (CFG::Visuals_RemoveFire) { /* suppress particles or effects */ }
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
        bool isLocal = (i == localIndex);
        bool drawESP = false;
        bool drawSkeleton = false;
        bool isPlayer = false;
        bool isBuilding = false;
        bool isPickup = false;
        bool isFlag = false;
        std::string name = "";
        int health = 0;
        int max_health = 100;
        Color_t clr = Color_t(255, 255, 255, 255);
        bool drawBacktrackSkeleton = false;
        C_TFPlayer* pPlayer = nullptr;
        if (strcmp(networkName, "CTFPlayer") == 0)
        {
            pPlayer = static_cast<C_TFPlayer*>(pBase);
            if (!pPlayer) continue;
            if (pPlayer->m_lifeState() != LIFE_ALIVE) continue;
            isPlayer = true;
            bool isCloaked = InCond(pPlayer, 4);
            if (isCloaked && (CFG::ESP_HideCloaked || CFG::ESP_SkeletonHideCloaked)) {
                if ((CFG::ESP_HideCloaked && CFG::ESP_SkeletonHideCloaked) || (!CFG::ESP_SkeletonLocalPlayer)) continue;
            }
            clr = isLocal ? Color_t(255, 255, 255, 255) : (team == TF_TEAM_RED ? Color_t(255, 0, 0, 255) : Color_t(0, 0, 255, 255));
            drawESP = (isLocal ? CFG::ESP_LocalPlayer : true) && !(CFG::ESP_Team && isTeammate) && !(isCloaked && CFG::ESP_HideCloaked);
            drawSkeleton = (isLocal ? CFG::ESP_SkeletonLocalPlayer : CFG::ESP_Skeleton) && !(CFG::ESP_SkeletonTeam && isTeammate) && !(isCloaked && CFG::ESP_SkeletonHideCloaked);
            // Check for thirdperson for local player
            if (isLocal) {
                if (!CFG::Misc_ThirdPerson_Enable) {
                    drawESP = false;
                    drawSkeleton = false;
                }
            }
            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(i, &info)) name = info.name ? info.name : "unknown";
            if (pResource) {
                static int healthOffset = NetVars::GetNetVar("CPlayerResource", "m_iHealth");
                static int maxHealthOffset = NetVars::GetNetVar("CTFPlayerResource", "m_iMaxHealth");
                health = *reinterpret_cast<int*>(
                    reinterpret_cast<uintptr_t>(pResource) + healthOffset + i * sizeof(int)
                    );
                max_health = *reinterpret_cast<int*>(
                    reinterpret_cast<uintptr_t>(pResource) + maxHealthOffset + i * sizeof(int)
                    );
            }
            if (max_health <= 0) max_health = pPlayer->GetMaxHealth();
            if (max_health <= 0) max_health = 100;
            health = std::clamp(health, 0, max_health);
            // Backtrack check
            int backtrackType = CFG::ESP_Skeleton_BacktrackType; // Combined
            bool backtrackCondition = false;
            if (backtrackType & (1 << 3)) backtrackCondition = true; // All
            else {
                if (isEnemy && (backtrackType & (1 << 0))) backtrackCondition = true;
                if (isTeammate && (backtrackType & (1 << 1))) backtrackCondition = true;
                if (isLocal && (backtrackType & (1 << 2))) backtrackCondition = true;
            }
            drawBacktrackSkeleton = CFG::ESP_Skeleton_Backtrack && backtrackCondition;
        }
        else if (strcmp(networkName, "CObjectSentrygun") == 0 || strcmp(networkName, "CObjectDispenser") == 0 || strcmp(networkName, "CObjectTeleporter") == 0)
        {
            if (!CFG::ESP_Build && !CFG::ESP_SkeletonBuild) continue;
            isBuilding = true;
            if (CFG::ESP_BuildOnlyEnemy && !isEnemy) continue;
            clr = (team == TF_TEAM_RED ? Color_t(255, 0, 0, 255) : Color_t(0, 0, 255, 255));
            if (strcmp(networkName, "CObjectSentrygun") == 0) name = "Sentry";
            else if (strcmp(networkName, "CObjectDispenser") == 0) name = "Dispenser";
            else name = "Teleporter";
            int level = *reinterpret_cast<int*>(
                reinterpret_cast<uintptr_t>(pBase) + NetVars::GetNetVar("CBaseObject", "m_iUpgradeLevel")
                );
            name += " Lvl " + std::to_string(level);
            health = static_cast<C_BaseObject*>(pBase)->m_iHealth();
            max_health = static_cast<C_BaseObject*>(pBase)->m_iMaxHealth();
            drawESP = CFG::ESP_Build && !(CFG::ESP_BuildOnlyEnemy && !isEnemy);
            drawSkeleton = CFG::ESP_SkeletonBuild && !(CFG::ESP_SkeletonBuildOnlyEnemy && !isEnemy);
        }
        else if ((strcmp(networkName, "CTFAmmoPack") == 0 ||
            strstr(networkName, "item_healthkit_") ||
            strstr(networkName, "item_ammopack_")) ||
            (strcmp(networkName, "CBaseAnimating") == 0 &&
                (H::Entities->IsHealthPack(pBase) || H::Entities->IsAmmoPack(pBase))))
        {
            // Skip se for debris de building (ex: sentry broken -> debris model similar a ammo, mas class != CTFAmmoPack e sem strstr)
            if (strstr(networkName, "debris") && !strstr(networkName, "item_")) continue;
            if (!CFG::ESP_Pickups) continue; // Removed skeleton
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
            drawSkeleton = false;
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
            // Chams e Skeleton independentes
            drawSkeleton = false;
            clr = Color_t(255, 255, 0, 255); // yellow
        }
        else continue;
        Vec3 origin = pBase->GetAbsOrigin();
        float dist = (pLocal->GetShootPos() - origin).Length();
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
        if (drawESP || drawSkeleton) { // Desenhar mesmo se só chams/skeleton
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
            if (draw_box && drawESP) { // Box só se drawESP
                int boxLeft = static_cast<int>(std::round(left));
                int boxTop = static_cast<int>(std::round(top));
                int boxW = width;
                int boxH = height;
                bool useAA = (dist < 1500.0f);
                switch (CFG::ESP_BoxType) {
                case 0: // 2D
                    DrawBox2D(boxLeft, boxTop, boxW, boxH, clr);
                    break;
                case 1: // 3D
                    DrawBox3D(screenPts, clr, useAA);
                    break;
                case 2: // Corner
                    DrawBoxCorner(boxLeft, boxTop, boxW, boxH, clr);
                    break;
                default:
                    break;
                }
            }
            if (draw_name && !name.empty())
            {
                const CFont& font = H::Fonts->Get(EFonts::ESP);
                H::Draw->String(font, static_cast<int>((left + right) / 2.0f), static_cast<int>(top) - 15, Color_t(255, 255, 255, 255), POS_CENTERX, name.c_str());
            }
            if (CFG::ESP_Health && max_health > 0 && (isPlayer || isBuilding))
            {
                health = std::clamp(health, 0, max_health);
                int healthType = CFG::ESP_HealthType;
                int barX = static_cast<int>(std::round(left)) - 6;
                int barY = static_cast<int>(std::round(top)) - 1;
                int barW = 4;
                int barH = height + 2;
                bool hasBar = (healthType == 0 || healthType == 2);
                if (hasBar) {
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
                if (healthType == 1 || healthType == 2) {
                    const CFont& font = H::Fonts->Get(EFonts::ESP);
                    std::string healthStr = std::to_string(health);
                    int textW = font.GetStringWidth(healthStr.c_str());
                    int textH = H::Fonts->GetFontHeight(EFonts::ESP);
                    int textX;
                    if (hasBar) {
                        textX = barX - textW - 2;
                    }
                    else {
                        textX = static_cast<int>(left) - textW - 2;
                    }
                    int textY = static_cast<int>(top);
                    H::Draw->String(font, textX, textY, Color_t(255, 255, 255, 255), POS_DEFAULT, healthStr.c_str());
                }
            }
        }
        if (isPlayer && drawESP) {
            int playerClass = pPlayer->m_iClass();
            // ESP Conds (inspired by Amalgam and Seowned implementations)
            if (CFG::ESP_Conds) {
                std::vector<std::string> conds;
                if (InCond(pPlayer, 4)) conds.push_back("Cloaked");
                if (InCond(pPlayer, 3)) conds.push_back("Disguised");
                if (InCond(pPlayer, 5)) conds.push_back("Uber");
                if (InCond(pPlayer, 22)) conds.push_back("Burning");
                if (InCond(pPlayer, 7)) conds.push_back("Taunt");
                // Add more conditions as needed (e.g., Bonked, Jarated, etc.)
                const CFont& font = H::Fonts->Get(EFonts::ESP);
                int condX = static_cast<int>(right) + 5;
                int condY = static_cast<int>(top);
                for (const auto& c : conds) {
                    H::Draw->String(font, condX, condY, Color_t(255, 255, 255, 255), POS_DEFAULT, c.c_str());
                    condY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                }
            }
            // ESP Uber and Uber Bar for Medics (inspired by Amalgam)
            if (playerClass == 5 && (CFG::ESP_Uber || CFG::ESP_UberBar)) {
                C_WeaponMedigun* pMedigun = nullptr;
                C_BaseEntity* pWeaponEnt = pPlayer->m_hActiveWeapon().Get();
                if (pWeaponEnt) {
                    C_TFWeaponBase* pWeapon = static_cast<C_TFWeaponBase*>(pWeaponEnt);
                    if (pWeapon && pWeapon->GetWeaponID() == 29) {
                        pMedigun = static_cast<C_WeaponMedigun*>(pWeapon);
                    }
                }
                if (!pMedigun) {
                    // Check secondary slot
                    C_TFWeaponBase* pSecondary = pPlayer->GetWeaponFromSlot(1);
                    if (pSecondary && pSecondary->GetWeaponID() == 29) {
                        pMedigun = static_cast<C_WeaponMedigun*>(pSecondary);
                    }
                }
                if (pMedigun) {
                    float charge = pMedigun->m_flChargeLevel();
                    const CFont& font = H::Fonts->Get(EFonts::ESP);
                    if (CFG::ESP_Uber) {
                        std::string uberText = "Uber: " + std::to_string(static_cast<int>(charge * 100.0f)) + "%";
                        int uberY = static_cast<int>(bottom) + 5;
                        H::Draw->String(font, static_cast<int>((left + right) / 2.0f), uberY, Color_t(255, 0, 255, 255), POS_CENTERX, uberText.c_str());
                    }
                    if (CFG::ESP_UberBar) {
                        int barX = static_cast<int>(left) - 12;
                        int barY = static_cast<int>(top) - 1;
                        int barW = 4;
                        int barH = height + 2;
                        H::Draw->Rect(barX, barY, barW, barH, Color_t(0, 0, 0, 200));
                        if (barH > 2 && charge > 0.0f) {
                            int fillH = static_cast<int>(std::round((barH - 2) * charge));
                            int fillY = barY + (barH - 1) - fillH;
                            Color_t uberClr(255, 0, 255, 255); // Purple for uber
                            H::Draw->Rect(barX + 1, fillY, 2, fillH, uberClr);
                        }
                    }
                }
            }
            // ESP Sniper Lines (inspired by Seowned and Amalgam - draw aim projection lines for enemy snipers)
            if (CFG::ESP_SniperLines && isEnemy && playerClass == 2) {
                Vec3 eyePos = pPlayer->GetShootPos();
                Vec3 ang = pPlayer->GetEyeAngles();
                Vec3 fwd;
                Math::AngleVectors(ang, &fwd);
                Vec3 end = eyePos + fwd * 16384.0f; // Long distance
                // Trace to wall (optional, for realism)
                CGameTrace tr;
                CTraceFilterWorldAndPropsOnly filter;
                Ray_t ray;
                ray.Init(eyePos, end);
                I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
                end = tr.endpos;
                Vec3 scrStart, scrEnd;
                if (H::Draw->W2S(eyePos, scrStart) && H::Draw->W2S(end, scrEnd)) {
                    DrawSmoothBoneLine(scrStart, scrEnd, clr, true);
                }
            }
            // ESP Tracer (inspired by Seowned - draw tracers from crosshair to enemy players)
            if (CFG::ESP_Tracer && isEnemy && !anyFailed) {
                Vec3 head = GetHitboxPosition(pPlayer, 0);
                Vec3 scrHead;
                if (H::Draw->W2S(head, scrHead)) {
                    int sw, sh;
                    I::EngineClient->GetScreenSize(sw, sh);
                    Vec3 center(static_cast<float>(sw) / 2.0f, static_cast<float>(sh) / 2.0f, 0.0f);
                    DrawSmoothBoneLine(center, scrHead, clr, true);
                }
            }
        }
        if (CFG::ESP_Offscreen && isEnemy && anyFailed && isPlayer) {
            DrawOffscreenArrow(origin, clr);
        }
        if ((drawSkeleton || drawBacktrackSkeleton) && (isPlayer || isBuilding)) // Removed isFlag and isPickup
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
            int numBones = std::min(static_cast<int>(pHDR->numbones), maxBones);
            int hitboxSet = pAnimating->m_nHitboxSet();
            int hitboxCount = pHDR->iHitboxCount(hitboxSet);
            bool useAA = (dist < 1500.0f);
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
                            DrawSmoothBoneLine(it1->second, it2->second, boneColor, useAA); // Use smooth lines with adjusted alpha
                        }
                    }
                }
            }
            // Backtrack drawing
            if (isPlayer && (drawBacktrackSkeleton)) {
                int totalRecords = 0;
                if (F::LagRecords->HasRecords(pPlayer, &totalRecords)) {
                    for (int n = 1; n <= totalRecords; ++n) { // start from 1 to skip current
                        auto pRecord = F::LagRecords->GetRecord(pPlayer, n);
                        if (!pRecord || !pRecord->Player)
                            continue;
                        F::LagRecordMatrixHelper->Set(pRecord);
                        // Recompute boneMatrix for backtrack record (fix: call SetupBones again with record's simtime)
                        ok = pClientEnt->SetupBones(
                            boneMatrix,
                            maxBones,
                            BONE_USED_BY_ANYTHING,
                            pRecord->SimulationTime
                        );
                        if (!ok) continue;
                        float alphaFactor = 1.0f - static_cast<float>(n) / static_cast<float>(totalRecords + 1);
                        Color_t fadedClr = clr;
                        fadedClr.a = static_cast<unsigned char>(clr.a * alphaFactor);
                        // Draw backtrack skeleton
                        if (drawBacktrackSkeleton) {
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
                            Color_t boneColor = fadedClr;
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
                                        DrawSmoothBoneLine(it1->second, it2->second, boneColor, useAA);
                                    }
                                }
                            }
                        }
                        F::LagRecordMatrixHelper->Restore();
                    }
                }
            }
        }
    }
    if (CFG::Aimbot_DrawFOV) {
        DrawFOVCircle(CFG::Aimbot_FOV, Color_t(0, 0, 255, 255)); // blue for aimbot
        DrawFOVCircle(CFG::Aimbot_Projectile_FOV, Color_t(0, 255, 0, 255)); // green for project
        DrawFOVCircle(CFG::Aimbot_Melee_FOV, Color_t(255, 0, 0, 255)); // red for melee
    }
}
CESP gESP;