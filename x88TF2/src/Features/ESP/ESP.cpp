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
#define TF_COND_STEALTHED 7
static bool InCond(C_TFPlayer* pPlayer, int cond)
{
    return (pPlayer->m_nPlayerCond() & (1 << cond)) != 0;
}
static bool IsFiniteVec(const Vec3& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
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
void DrawSmoothBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    if (!H::Draw) return;
    int ax = static_cast<int>(std::round(a.x));
    int ay = static_cast<int>(std::round(a.y));
    int bx = static_cast<int>(std::round(b.x));
    int by = static_cast<int>(std::round(b.y));
    // Draw main line
    H::Draw->Line(ax, ay, bx, by, clr);
    // Draw offset lines for anti-aliasing effect with lower alpha to make thinner appearance
    Color_t aaClr = Color_t(clr.r, clr.g, clr.b, 30); // Adjusted alpha for subtle shine
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
void DrawScreenLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    DrawSmoothBoneLine(a, b, clr);
}
// --- Helper: desenha apenas o wireframe projetado (sem pontos/glow) ---
static void DrawProjectedHitboxWire(const Vec3 proj[8], const Color_t& clr, bool withGlow = false)
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
    if (withGlow)
    {
        Color_t glowClr = Color_t(255, 180, 0, 10); // Warmer glow color (orange-ish) with slightly higher alpha for more shine
        for (auto& e : edges)
        {
            Vec3 A = proj[e.first];
            Vec3 B = proj[e.second];
            float dx = B.x - A.x;
            float dy = B.y - A.y;
            int off = 1;
            if (std::abs(dx) > std::abs(dy)) {
                H::Draw->Line(static_cast<int>(std::round(A.x)), static_cast<int>(std::round(A.y - off)),
                    static_cast<int>(std::round(B.x)), static_cast<int>(std::round(B.y - off)),
                    glowClr);
                H::Draw->Line(static_cast<int>(std::round(A.x)), static_cast<int>(std::round(A.y + off)),
                    static_cast<int>(std::round(B.x)), static_cast<int>(std::round(B.y + off)),
                    glowClr);
            }
            else {
                H::Draw->Line(static_cast<int>(std::round(A.x - off)), static_cast<int>(std::round(A.y)),
                    static_cast<int>(std::round(B.x - off)), static_cast<int>(std::round(B.y)),
                    glowClr);
                H::Draw->Line(static_cast<int>(std::round(A.x + off)), static_cast<int>(std::round(A.y)),
                    static_cast<int>(std::round(B.x + off)), static_cast<int>(std::round(B.y)),
                    glowClr);
            }
        }
    }
    for (auto& e : edges)
    {
        const Vec3& A = proj[e.first];
        const Vec3& B = proj[e.second];
        DrawSmoothBoneLine(A, B, clr); // Use smooth lines with adjusted alpha
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
    Vec3 localEye = pLocal->GetEyePosition();
    Vec3 dir = origin - localEye;
    dir.Normalize();
    Vec3 viewAngles;
    I::EngineClient->GetViewAngles(viewAngles);
    float angle = atan2(dir.y, dir.x) - viewAngles.y * PI / 180; // remove + PI/2 to fix direction
    float radius = std::min(screenW, screenH) / 2.0f - 50.0f; // edge
    float centerX = screenW / 2.0f;
    float centerY = screenH / 2.0f;
    Vec3 arrowTip(centerX + radius * cos(angle), centerY + radius * sin(angle), 0);
    float sideAngle = PI / 6; // 30 deg
    Vec3 side1(centerX + (radius - 20) * cos(angle + sideAngle), centerY + (radius - 20) * sin(angle + sideAngle), 0);
    Vec3 side2(centerX + (radius - 20) * cos(angle - sideAngle), centerY + (radius - 20) * sin(angle - sideAngle), 0);
    DrawSmoothBoneLine(arrowTip, side1, clr);
    DrawSmoothBoneLine(arrowTip, side2, clr);
    DrawSmoothBoneLine(side1, side2, clr);
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
    const int maxClients = I::EngineClient->GetMaxClients();
    const int highestIndex = I::ClientEntityList->GetHighestEntityIndex();
    const int end = std::max(maxClients, highestIndex);
    int localTeam = -1;
    pLocal->IsInValidTeam(&localTeam);
    int localIndex = I::EngineClient->GetLocalPlayer();
    auto pResource = GetTFPlayerResource();
    for (int i = 0; i <= end; ++i)
    {
        IClientEntity* pClientEnt = I::ClientEntityList->GetClientEntity(i);
        if (!pClientEnt) continue;
        C_BaseEntity* pBase = pClientEnt->As<C_BaseEntity>();
        if (!pBase) continue;
        if (pClientEnt->IsDormant()) continue;
        int classID = static_cast<int>(pBase->GetClassId());
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
        if (classID == static_cast<int>(ETFClassIds::CTFPlayer))
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
        else if (classID == static_cast<int>(ETFClassIds::CObjectSentrygun) || classID == static_cast<int>(ETFClassIds::CObjectDispenser) || classID == static_cast<int>(ETFClassIds::CObjectTeleporter))
        {
            if (!CFG::ESP_Build && !CFG::ESP_ChamsBuild && !CFG::ESP_SkeletonBuild) continue;
            isBuilding = true;
            if (CFG::ESP_BuildOnlyEnemy && !isEnemy) continue;
            clr = (team == TF_TEAM_RED ? Color_t(255, 0, 0, 255) : Color_t(0, 0, 255, 255));
            if (classID == static_cast<int>(ETFClassIds::CObjectSentrygun)) name = "Sentry";
            else if (classID == static_cast<int>(ETFClassIds::CObjectDispenser)) name = "Dispenser";
            else name = "Teleporter";
            int level = *reinterpret_cast<int*>((uintptr_t)pBase + NetVars::GetNetVar("CBaseObject", "m_iUpgradeLevel"));
            name += " Lvl " + std::to_string(level);
            health = static_cast<C_BaseObject*>(pBase)->m_iHealth();
            max_health = static_cast<C_BaseObject*>(pBase)->m_iMaxHealth();
            drawESP = CFG::ESP_Build && !(CFG::ESP_BuildOnlyEnemy && !isEnemy);
            drawSkeleton = CFG::ESP_SkeletonBuild && !(CFG::ESP_SkeletonBuildOnlyEnemy && !isEnemy);
            drawChamsBox = CFG::ESP_ChamsBuild && !(CFG::ESP_ChamsBuildOnlyEnemy && !isEnemy);
        }
        else if (classID == static_cast<int>(ETFClassIds::CTFAmmoPack) ||
            strstr(pBase->GetClientClass()->m_pNetworkName, "item_healthkit_") ||
            strstr(pBase->GetClientClass()->m_pNetworkName, "item_ammopack_"))
        {
            if (!CFG::ESP_Pickups) continue;
            isPickup = true;
            if (classID == static_cast<int>(ETFClassIds::CTFAmmoPack) || strstr(pBase->GetClientClass()->m_pNetworkName, "ammopack")) {
                name = "Ammo";
                clr = Color_t(255, 215, 0, 255); // gold
            }
            else {
                name = "Medkit";
                clr = Color_t(0, 255, 0, 255); // green
            }
            drawESP = true;
            drawSkeleton = false;
            drawChamsBox = false;
        }
        else if (strcmp(pBase->GetClientClass()->m_pNetworkName, "CTFItemTeamFlag") == 0 ||
            strcmp(pBase->GetClientClass()->m_pNetworkName, "CItemTeamFlag") == 0 ||
            strcmp(pBase->GetClientClass()->m_pNetworkName, "item_teamflag") == 0)
        {
            if (!CFG::ESP_CaptureFlag) continue;
            isFlag = true;
            name = "Flag";
            drawESP = CFG::ESP_CaptureFlag;
            drawSkeleton = CFG::ESP_SkeletonCaptureFlag;
            drawChamsBox = CFG::ESP_ChamsCaptureFlag;
            clr = Color_t(255, 255, 0, 255); // yellow
        }
        else if (strcmp(pBase->GetClientClass()->m_pNetworkName, "CTeamControlPoint") == 0)
        {
            if (!CFG::ESP_CaptureFlag) continue;
            isFlag = true;
            name = "Control Point";
            drawESP = CFG::ESP_CaptureFlag;
            drawSkeleton = CFG::ESP_SkeletonCaptureFlag;
            drawChamsBox = CFG::ESP_ChamsCaptureFlag;
            clr = Color_t(255, 255, 0, 255); // yellow
        }
        else if (strcmp(pBase->GetClientClass()->m_pNetworkName, "CFuncTrackTrain") == 0)
        {
            const model_t* model = pBase->GetModel();
            if (model)
            {
                const char* mdlName = I::ModelInfoClient->GetModelName(model);
                if (strstr(mdlName, "cart") || strstr(mdlName, "train"))
                {
                    if (!CFG::ESP_CaptureFlag) continue;
                    isFlag = true;
                    name = "Payload Cart";
                    drawESP = CFG::ESP_CaptureFlag;
                    drawSkeleton = CFG::ESP_SkeletonCaptureFlag;
                    drawChamsBox = CFG::ESP_ChamsCaptureFlag;
                    clr = Color_t(255, 255, 0, 255); // yellow
                }
            }
        }
        else continue;
        Vec3 origin = pBase->GetAbsOrigin();
        Vec3 mins = pBase->m_vecMins();
        Vec3 maxs = pBase->m_vecMaxs();
        if (!IsFiniteVec(mins) || !IsFiniteVec(maxs) || !IsFiniteVec(origin)) continue;
        if (mins.Length() == 0.0f && maxs.Length() == 0.0f) {
            if (isPickup) {
                mins = Vec3(-10, -10, 0);
                maxs = Vec3(10, 10, 20);
            }
            else if (isFlag) {
                mins = Vec3(-20, -20, 0);
                maxs = Vec3(20, 20, 80);
            }
            else continue;
        }
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
        if (drawESP) {
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
            if (draw_box)
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
                    for (int dy = 0; dy < barH; dy++)
                    {
                        int y = barY + dy;
                        if (y >= fillY && y < fillY + fillH)
                        {
                            float posRatio = static_cast<float>(dy) / static_cast<float>(barH - 1);
                            int red = static_cast<int>(255 * posRatio);
                            int green = static_cast<int>(255 * (1.0f - posRatio));
                            Color_t c(static_cast<unsigned char>(red), static_cast<unsigned char>(green), 0, 255);
                            H::Draw->Rect(barX + 1, y, 2, 1, c);
                        }
                    }
                }
            }
        }
        if (CFG::ESP_Offscreen && isEnemy && anyFailed && isPlayer) {
            DrawOffscreenArrow(origin, clr);
        }
        if ((drawSkeleton || drawChamsBox) && (isPlayer || isBuilding || isFlag))
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
                            for (int c = 0; c < 8; ++c)
                                Math::VectorTransform(modelHeadCorners[c], boneMatrix[headBoneIdx], headCorners[c]);
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
                Color_t wireColor = clr;
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
                    DrawProjectedHitboxWire(scr, wireColor, false); // No glow
                }
            }
        }
    }
}
CESP gESP;