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
#include <cstdint> // for uintptr_t
#define PI 3.14159265358979323846f
#define DEG2RAD(deg) ((deg) * PI / 180.0f)
C_BaseEntity * CESP::RainEntity = nullptr;
IClientNetworkable* CESP::RainNetworkable = nullptr;
C_BaseEntity* CESP::WindEntity = nullptr;
IClientNetworkable* CESP::WindNetworkable = nullptr;
static bool IsFiniteVec(const Vec3& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
static bool InCond(C_TFPlayer* pPlayer, int cond)
{
    return (pPlayer->m_nPlayerCond() & (1 << cond)) != 0;
}
static Color_t ColorLerp(const Color_t& a, const Color_t& b, float t)
{
    float fr = static_cast<float>(a.r) + (static_cast<float>(b.r) - static_cast<float>(a.r)) * t;
    float fg = static_cast<float>(a.g) + (static_cast<float>(b.g) - static_cast<float>(a.g)) * t;
    float fb = static_cast<float>(a.b) + (static_cast<float>(b.b) - static_cast<float>(a.b)) * t;
    float fa = static_cast<float>(a.a) + (static_cast<float>(b.a) - static_cast<float>(a.a)) * t;
    return Color_t(
        static_cast<unsigned char>(std::clamp(fr, 0.0f, 255.0f)),
        static_cast<unsigned char>(std::clamp(fg, 0.0f, 255.0f)),
        static_cast<unsigned char>(std::clamp(fb, 0.0f, 255.0f)),
        static_cast<unsigned char>(std::clamp(fa, 0.0f, 255.0f))
    );
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
void CESP::Rain()
{
    constexpr auto PRECIPITATION_INDEX = (MAX_EDICTS - 1);
    if (!CFG::Visuals_Rain)
    {
        if (RainEntity && RainEntity->GetClientNetworkable())
        {
            static const auto dwOff = NetVars::GetNetVar("CPrecipitation", "m_nPrecipType");
            *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(RainEntity) + dwOff) = 0;
            RainEntity->m_vecMins() = Vec3();
            RainEntity->m_vecMaxs() = Vec3();
            RainEntity->GetClientNetworkable()->PreDataUpdate(DATA_UPDATE_CREATED);
            RainEntity->GetClientNetworkable()->OnPreDataChanged(DATA_UPDATE_CREATED);
            RainEntity->GetClientNetworkable()->OnDataChanged(DATA_UPDATE_CREATED);
            RainEntity->GetClientNetworkable()->PostDataUpdate(DATA_UPDATE_CREATED);
        }
        return;
    }
    static ClientClass* pPrecipClass = nullptr;
    if (!pPrecipClass)
    {
        for (auto pReturn = I::BaseClientDLL->GetAllClasses(); pReturn; pReturn = pReturn->m_pNext)
        {
            if (pReturn->m_ClassID == static_cast<int>(ETFClassIds::CPrecipitation))
            {
                pPrecipClass = pReturn;
                break;
            }
        }
    }
    const auto* pRainEntity = I::ClientEntityList->GetClientEntity(PRECIPITATION_INDEX);
    if (!pRainEntity)
    {
        if (!pPrecipClass || !pPrecipClass->m_pCreateFn)
            return;
        RainNetworkable = reinterpret_cast<IClientNetworkable * (__cdecl*)(int, int)>(pPrecipClass->m_pCreateFn)(PRECIPITATION_INDEX, 0);
        if (!RainNetworkable)
            return;
        RainEntity = static_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(PRECIPITATION_INDEX));
        if (!RainEntity || !RainEntity->GetClientNetworkable())
            return;
    }
    else if (!RainEntity)
    {
        RainEntity = static_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(PRECIPITATION_INDEX));
        RainNetworkable = RainEntity ? RainEntity->GetClientNetworkable() : nullptr;
    }
    if (RainEntity && RainEntity->GetClientNetworkable())
    {
        static const auto dwOff = NetVars::GetNetVar("CPrecipitation", "m_nPrecipType");
        *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(RainEntity) + dwOff) = CFG::Visuals_Rain - 1;
        RainEntity->GetClientNetworkable()->PreDataUpdate(DATA_UPDATE_CREATED);
        RainEntity->GetClientNetworkable()->OnPreDataChanged(DATA_UPDATE_CREATED);
        RainEntity->m_vecMins() = Vec3(-32768.0f, -32768.0f, -32768.0f);
        RainEntity->m_vecMaxs() = Vec3(32768.0f, 32768.0f, 32768.0f);
        RainEntity->GetClientNetworkable()->OnDataChanged(DATA_UPDATE_CREATED);
        RainEntity->GetClientNetworkable()->PostDataUpdate(DATA_UPDATE_CREATED);
    }
}
// Apenas desenha caixa (box ESP) — funções auxiliares simples
void CESP::DrawBox(int left, int top, int w, int h, const Color_t& clr)
{
    if (w <= 0 || h <= 0) return;
    H::Draw->OutlinedRect(left, top, w, h, clr);
}
void CESP::DrawBox2D(int left, int top, int w, int h, const Color_t& clr)
{
    Color_t black = Color_t(0, 0, 0, 255);
    H::Draw->OutlinedRect(left - 1, top - 1, w + 2, h + 2, black);
    H::Draw->OutlinedRect(left, top, w, h, clr);
    H::Draw->OutlinedRect(left + 1, top + 1, w - 2, h - 2, black);
}
void CESP::DrawBox3D(Vec3 scr[8], const Color_t& clr, bool useAA)
{
    Color_t black = Color_t(0, 0, 0, 255);
    Vec3 offsetScr[8];
    for (int i = 0; i < 8; ++i) {
        offsetScr[i] = scr[i];
        offsetScr[i].x -= 1.0f;
        offsetScr[i].y -= 1.0f;
    }
    DrawProjectedHitboxWire(offsetScr, black, useAA);
    for (int i = 0; i < 8; ++i) {
        offsetScr[i] = scr[i];
        offsetScr[i].x += 1.0f;
        offsetScr[i].y += 1.0f;
    }
    DrawProjectedHitboxWire(offsetScr, black, useAA);
    DrawProjectedHitboxWire(scr, clr, useAA);
}
void CESP::DrawBoxCorner(int left, int top, int w, int h, const Color_t& clr)
{
    Color_t black = Color_t(0, 0, 0, 255);
    int cornerLen = std::min(w, h) / 5;
    if (cornerLen < 1) return;
    // Black outline lines (outside)
    // Top-left
    H::Draw->Line(left - 1, top - 1, left + cornerLen, top - 1, black);
    H::Draw->Line(left - 1, top - 1, left - 1, top + cornerLen, black);
    H::Draw->Line(left + 1, top + 1, left + cornerLen, top + 1, black);
    H::Draw->Line(left + 1, top + 1, left + 1, top + cornerLen, black);
    // Top-right
    H::Draw->Line(left + w - cornerLen, top - 1, left + w + 1, top - 1, black);
    H::Draw->Line(left + w + 1, top - 1, left + w + 1, top + cornerLen, black);
    H::Draw->Line(left + w - cornerLen, top + 1, left + w - 1, top + 1, black);
    H::Draw->Line(left + w - 1, top + 1, left + w - 1, top + cornerLen, black);
    // Bottom-left
    H::Draw->Line(left - 1, top + h - cornerLen, left - 1, top + h + 1, black);
    H::Draw->Line(left - 1, top + h + 1, left + cornerLen, top + h + 1, black);
    H::Draw->Line(left + 1, top + h - cornerLen, left + 1, top + h - 1, black);
    H::Draw->Line(left + 1, top + h - 1, left + cornerLen, top + h - 1, black);
    // Bottom-right
    H::Draw->Line(left + w - cornerLen, top + h + 1, left + w + 1, top + h + 1, black);
    H::Draw->Line(left + w + 1, top + h - cornerLen, left + w + 1, top + h + 1, black);
    H::Draw->Line(left + w - cornerLen, top + h - 1, left + w - 1, top + h - 1, black);
    H::Draw->Line(left + w - 1, top + h - cornerLen, left + w - 1, top + h - 1, black);
    // Inner colored lines
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
    // Removed AA for thinner lines
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
        DrawOutlinedLine(scr[e.first], scr[e.second], clr, CFG::Color_ESP_Outline);
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
        DrawThinLine(A, B, clr);
    }
}

void CESP::PlayerArrow(C_TFPlayer* Player, Color_t Clr) {
    if (!Player || Player->IsDormant()) return;
    auto get_clockwise_angle = [&](const Vec3& view_angles, const Vec3& aim_angle) -> float {
        Vec3 angle, aim;
        Math::AngleVectors(view_angles, &angle);
        Math::AngleVectors(aim_angle, &aim);
        return -atan2f(angle.x * aim.y - angle.y * aim.x, angle.x * angle.x + angle.y * aim.y);
        };
    Vec3 viewAngles;
    I::EngineClient->GetViewAngles(viewAngles);
    const Vec3 angle_to = Math::CalcAngle(H::Entities->GetLocal()->GetAbsOrigin(), Player->GetAbsOrigin());
    const float degrees = get_clockwise_angle(viewAngles, angle_to);
    int sw, sh;
    I::EngineClient->GetScreenSize(sw, sh);
    const float aspect = static_cast<float>(sw) / static_cast<float>(sh);
    const float arrows_distance = CFG::ESP_Offscreen_Radius / 1000.0f;
    const bool arrows_shape = true;
    const float x1 = (sw * arrows_distance * (arrows_shape ? aspect : 1.0f) + 5.0f) * cos(degrees - PI / 2);
    const float y1 = (sw * arrows_distance + 5.0f) * sin(degrees - PI / 2);
    const float x2 = (sw * arrows_distance * (arrows_shape ? aspect : 1.0f) + 15.0f) * cos(degrees - PI / 2);
    const float y2 = (sw * arrows_distance + 15.0f) * sin(degrees - PI / 2);
    // arrow sizing
    const float arrow_length = 25.0f; // distance from tip to base
    const float arrow_width = 22.0f; // base width of triangle
    // direction from base-ish to tip
    Vec3 dir = Vec3(x2 - x1, y2 - y1, 0.0f);
    float dirLen = dir.Length();
    if (dirLen <= 0.0001f) return;
    Vec3 unitDir = dir / dirLen;
    // robust base point: move back from tip by arrow_length along unitDir
    Vec3 tip = Vec3(x2, y2, 0.0f);
    Vec3 base = tip - unitDir * arrow_length;
    // perpendicular vector for base width
    Vec3 perp = Vec3(-unitDir.y, unitDir.x, 0.0f);
    Vec3 left = base + perp * (arrow_width * 0.5f);
    Vec3 right = base - perp * (arrow_width * 0.5f);
    const float center_x = sw * 0.5f;
    const float center_y = sh * 0.5f;
    const Color_t outlineColor = Color_t(0, 0, 0, 255);
    Color_t shapeColor = Clr;
    int style = CFG::ESP_Offscreen_Style;
    bool filled = CFG::ESP_Offscreen_Filled;
    if (style == 0) { // Triangle
        Vertex_t triangle[3];
        triangle[0].Init(Vector2D(center_x + tip.x, center_y + tip.y)); // apex (tip)
        triangle[1].Init(Vector2D(center_x + left.x, center_y + left.y)); // base left
        triangle[2].Init(Vector2D(center_x + right.x, center_y + right.y)); // base right
        if (filled) {
            H::Draw->Polygon(3, triangle, shapeColor);
        }
        else {
            H::Draw->Line(static_cast<int>(triangle[0].m_Position.x), static_cast<int>(triangle[0].m_Position.y),
                static_cast<int>(triangle[1].m_Position.x), static_cast<int>(triangle[1].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(triangle[1].m_Position.x), static_cast<int>(triangle[1].m_Position.y),
                static_cast<int>(triangle[2].m_Position.x), static_cast<int>(triangle[2].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(triangle[2].m_Position.x), static_cast<int>(triangle[2].m_Position.y),
                static_cast<int>(triangle[0].m_Position.x), static_cast<int>(triangle[0].m_Position.y), shapeColor);
        }
    }
    else if (style == 1) { // Circle
        const float circle_radius = 12.0f; // Arbitrary size
        Vec3 circle_center = (tip + left + right) / 3.0f; // Centroid for placement
        int cx = static_cast<int>(center_x + circle_center.x);
        int cy = static_cast<int>(center_y + circle_center.y);
        if (filled) {
            H::Draw->FilledCircle(cx, cy, circle_radius, 32, shapeColor);
        }
        else {
            H::Draw->OutlinedCircle(cx, cy, circle_radius, 32, shapeColor);
        }
    }
    else if (style == 2) { // Horizontal bar (tangential to the circle)
        const float bar_length = 37.0f;
        const float bar_width = 8.0f;
        const float half_length = bar_length * 0.5f;
        Vec3 bar_center = tip;
        Vec3 left_end = bar_center + perp * half_length;
        Vec3 right_end = bar_center - perp * half_length;
        Vec3 inward = unitDir;
        Vec3 left_front = left_end + inward * (bar_width / 2.0f);
        Vec3 left_back = left_end - inward * (bar_width / 2.0f);
        Vec3 right_front = right_end + inward * (bar_width / 2.0f);
        Vec3 right_back = right_end - inward * (bar_width / 2.0f);
        Vertex_t quad[4];
        quad[0].Init(Vector2D(center_x + left_back.x, center_y + left_back.y));
        quad[1].Init(Vector2D(center_x + left_front.x, center_y + left_front.y));
        quad[2].Init(Vector2D(center_x + right_front.x, center_y + right_front.y));
        quad[3].Init(Vector2D(center_x + right_back.x, center_y + right_back.y));
        if (filled) {
            H::Draw->Polygon(4, quad, shapeColor);
        }
        else {
            H::Draw->Line(static_cast<int>(quad[0].m_Position.x), static_cast<int>(quad[0].m_Position.y),
                static_cast<int>(quad[1].m_Position.x), static_cast<int>(quad[1].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(quad[1].m_Position.x), static_cast<int>(quad[1].m_Position.y),
                static_cast<int>(quad[2].m_Position.x), static_cast<int>(quad[2].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(quad[2].m_Position.x), static_cast<int>(quad[2].m_Position.y),
                static_cast<int>(quad[3].m_Position.x), static_cast<int>(quad[3].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(quad[3].m_Position.x), static_cast<int>(quad[3].m_Position.y),
                static_cast<int>(quad[0].m_Position.x), static_cast<int>(quad[0].m_Position.y), shapeColor);
        }
    }
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
            clr = isLocal ? CFG::Color_Local : (team == TF_TEAM_RED ? CFG::Color_TeamRed : CFG::Color_TeamBlue);
            drawESP = (isLocal ? CFG::ESP_LocalPlayer : true) && !(CFG::ESP_Team && isTeammate && !isLocal) && !(isCloaked && CFG::ESP_HideCloaked);
            drawSkeleton = (isLocal ? CFG::ESP_SkeletonLocalPlayer : CFG::ESP_Skeleton) && !(CFG::ESP_SkeletonTeam && isTeammate && !isLocal) && !(isCloaked && CFG::ESP_SkeletonHideCloaked);
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
            if (health < 0) health = 0;
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
            clr = isTeammate ? CFG::Color_BuildingTeam : CFG::Color_BuildingEnemy;
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
                clr = CFG::Color_Ammo;
            }
            else {
                name = "Medkit";
                clr = CFG::Color_Medkit;
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
            clr = CFG::Color_Flag;
        }
        else continue;
        Vec3 origin = pBase->GetAbsOrigin();
        float dist = (pLocal->GetShootPos() - origin).Length() / 39.37f;
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
        Vec3 screen;
        bool onScreen = H::Draw->W2S(origin, screen);
        if (onScreen)
        {
            int sw, sh;
            I::EngineClient->GetScreenSize(sw, sh);
            onScreen = screen.x > 0 && screen.x < sw && screen.y > 0 && screen.y < sh;
        }
        if (!onScreen) {
            if (CFG::ESP_Offscreen && isEnemy && isPlayer && (CFG::ESP_Offscreen_MaxDist <= 0.0f || dist <= CFG::ESP_Offscreen_MaxDist)) {
                PlayerArrow(pPlayer, clr);
            }
            continue;
        }
        Vec3 screenPts[8];
        bool allProjected = true;
        for (int k = 0; k < 8; ++k)
        {
            if (!H::Draw->W2S(points[k], screenPts[k]))
            {
                allProjected = false;
            }
        }
        if (!allProjected) continue; // skip if any behind camera
        float left = std::numeric_limits<float>::max();
        float top = std::numeric_limits<float>::max();
        float right = std::numeric_limits<float>::min();
        float bottom = std::numeric_limits<float>::min();
        for (int k = 0; k < 8; ++k)
        {
            if (std::isfinite(screenPts[k].x) && std::isfinite(screenPts[k].y)) {
                if (screenPts[k].x < left) left = screenPts[k].x;
                if (screenPts[k].x > right) right = screenPts[k].x;
                if (screenPts[k].y < top) top = screenPts[k].y;
                if (screenPts[k].y > bottom) bottom = screenPts[k].y;
            }
        }
        if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) || !std::isfinite(bottom)) continue;
        int width = static_cast<int>(std::round(right - left));
        int height = static_cast<int>(std::round(bottom - top));
        if (width < 2 || height < 2) continue;
        int sw, sh;
        I::EngineClient->GetScreenSize(sw, sh);
        bool fullyOffscreen = (right < 0 || bottom < 0 || left > sw || top > sh);
        if (fullyOffscreen) {
            if (CFG::ESP_Offscreen && isEnemy && isPlayer && (CFG::ESP_Offscreen_MaxDist <= 0.0f || dist <= CFG::ESP_Offscreen_MaxDist)) {
                PlayerArrow(pPlayer, clr);
            }
            continue;
        }
        else {
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
                    Color_t nameClr = isBuilding ? CFG::Color_BuildingName : CFG::Color_Name;
                    H::Draw->String(font, static_cast<int>((left + right) / 2.0f), static_cast<int>(top) - 15, nameClr, POS_CENTERX, name.c_str());
                }
                if (CFG::ESP_Health && max_health > 0 && (isPlayer || isBuilding))
                {
                    int healthType = CFG::ESP_HealthType;
                    int barX = static_cast<int>(std::round(left)) - 6;
                    int barY = static_cast<int>(std::round(top)) - 1;
                    int barW = 4;
                    int barH = height + 2;
                    bool hasBar = (healthType == 0 || healthType == 2);
                    if (hasBar) {
                        H::Draw->Rect(barX, barY, barW, barH, CFG::Color_HealthBarBG);
                        if (barH > 2 && health > 0)
                        {
                            float ratio = static_cast<float>(health) / static_cast<float>(max_health);
                            int fillH = static_cast<int>(std::round((barH - 2) * std::min(1.0f, ratio)));
                            int fillY = barY + (barH - 1) - fillH;
                            Color_t c;
                            if (health > max_health) {
                                c = CFG::Color_Overheal;
                            }
                            else {
                                c = ColorLerp(CFG::Color_HealthLow, CFG::Color_HealthHigh, ratio);
                            }
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
                        H::Draw->String(font, textX, textY, CFG::Color_HealthText, POS_DEFAULT, healthStr.c_str());
                    }
                }
            }
            if (isPlayer && drawESP) {
                int playerClass = pPlayer->m_iClass();
                // ESP Conds (inspired by Amalgam and Seowned implementations)
                const CFont& font = H::Fonts->Get(EFonts::ESP);
                int rightTextX = static_cast<int>(right) + 5;
                int rightTextY = static_cast<int>(top);
                if (CFG::ESP_Conds) {
                    std::vector<std::string> conds;
                    if (InCond(pPlayer, 4)) conds.push_back("Cloaked");
                    if (InCond(pPlayer, 3)) conds.push_back("Disguised");
                    if (InCond(pPlayer, 5)) conds.push_back("Uber");
                    if (InCond(pPlayer, 22)) conds.push_back("Burning");
                    if (InCond(pPlayer, 7)) conds.push_back("Taunt");
                    // Add more conditions as needed (e.g., Bonked, Jarated, etc.)
                    for (const auto& c : conds) {
                        H::Draw->String(font, rightTextX, rightTextY, CFG::Color_CondsText, POS_DEFAULT, c.c_str());
                        rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                    }
                }
                if (CFG::ESP_Buffs || CFG::ESP_Debuffs) {
                    std::vector<std::string> buffs, debuffs;
                    // Buffs (positive)
                    if (InCond(pPlayer, 5)) buffs.push_back("Uber");
                    if (InCond(pPlayer, 11)) buffs.push_back("Kritz");
                    if (InCond(pPlayer, 16)) buffs.push_back("Banner");
                    if (InCond(pPlayer, 18)) buffs.push_back("Charging");
                    // Debuffs (negative)
                    if (InCond(pPlayer, 22)) debuffs.push_back("Burning");
                    if (InCond(pPlayer, 25)) debuffs.push_back("Bleeding");
                    if (InCond(pPlayer, 14)) debuffs.push_back("Bonked");
                    if (InCond(pPlayer, 21)) debuffs.push_back("Marked");
                    if (InCond(pPlayer, 28)) debuffs.push_back("Milk"); // Assuming cond 28 for milk
                    // Draw buffs if enabled
                    if (CFG::ESP_Buffs) {
                        for (const auto& b : buffs) {
                            H::Draw->String(font, rightTextX, rightTextY, Color_t(0, 255, 0, 255), POS_DEFAULT, b.c_str());
                            rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                        }
                    }
                    // Draw debuffs if enabled
                    if (CFG::ESP_Debuffs) {
                        for (const auto& d : debuffs) {
                            H::Draw->String(font, rightTextX, rightTextY, Color_t(255, 0, 0, 255), POS_DEFAULT, d.c_str());
                            rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                        }
                    }
                }
                if (CFG::ESP_DistanceEnemy && isEnemy) {
                    std::string distStr = std::to_string(static_cast<int>(dist)) + " m";
                    if (CFG::ESP_DistancePosition == 0) {
                        H::Draw->String(font, rightTextX, rightTextY, Color_t(255, 255, 255, 255), POS_DEFAULT, distStr.c_str());
                        rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                    }
                    else {
                        H::Draw->String(font, static_cast<int>((left + right) / 2.0f), static_cast<int>(bottom) + 5, Color_t(255, 255, 255, 255), POS_CENTERX, distStr.c_str());
                    }
                }
                if (CFG::ESP_Ping) {
                    static int pingOffset = NetVars::GetNetVar("CPlayerResource", "m_iPing");
                    int ping = pResource ? *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + pingOffset + i * sizeof(int)) : 0;
                    std::string pingStr = std::to_string(ping) + " ms";
                    H::Draw->String(font, rightTextX, rightTextY, Color_t(255, 255, 255, 255), POS_DEFAULT, pingStr.c_str());
                    rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                }
                if (CFG::ESP_KRDPlayer) {
                    static int scoreOffset = NetVars::GetNetVar("CTFPlayerResource", "m_iScore");
                    static int deathsOffset = NetVars::GetNetVar("CTFPlayerResource", "m_iDeaths");
                    int kills = pResource ? *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + scoreOffset + i * sizeof(int)) : 0;
                    int deaths = pResource ? *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + deathsOffset + i * sizeof(int)) : 0;
                    float ratio = deaths > 0 ? static_cast<float>(kills) / deaths : kills;
                    Color_t krdColor = Color_t(255, 255, 255, 255);
                    if (ratio > 5.0f) krdColor = Color_t(0, 0, 255, 255); // blue suspicious
                    else if (ratio > 2.0f) krdColor = Color_t(0, 255, 0, 255); // green good
                    else if (ratio >= 1.0f) krdColor = Color_t(255, 255, 0, 255); // yellow medium
                    else krdColor = Color_t(255, 0, 0, 255); // red bad
                    std::string krdStr = "K/D: " + std::to_string(kills) + "/" + std::to_string(deaths);
                    H::Draw->String(font, rightTextX, rightTextY, krdColor, POS_DEFAULT, krdStr.c_str());
                    rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                }
                if (CFG::ESP_LagCompensation) {
                    int totalRecords = 0;
                    F::LagRecords->HasRecords(pPlayer, &totalRecords);
                    std::string lagStr = "LagComp: " + std::to_string(totalRecords) + " ticks";
                    H::Draw->String(font, rightTextX, rightTextY, Color_t(255, 255, 255, 255), POS_DEFAULT, lagStr.c_str());
                    rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
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
                    bool startOn = H::Draw->W2S(eyePos, scrStart);
                    bool endOn = H::Draw->W2S(end, scrEnd);
                    if (startOn && endOn) {
                        DrawSmoothBoneLine(scrStart, scrEnd, CFG::Color_SniperLine, false);
                    }
                    else if (startOn || endOn) {
                        Vec3 start = eyePos;
                        Vec3 dir = end - eyePos;
                        if (!startOn) {
                            start = end;
                            dir = eyePos - end;
                        }
                        float low = 0.0f;
                        float high = 1.0f;
                        for (int it = 0; it < 20; ++it) {
                            float mid = (low + high) / 2.0f;
                            Vec3 test = start + dir * mid;
                            Vec3 dummy;
                            if (H::Draw->W2S(test, dummy)) {
                                low = mid;
                            }
                            else {
                                high = mid;
                            }
                        }
                        Vec3 clipPos = start + dir * low;
                        if (startOn) {
                            scrEnd = Vec3();
                            H::Draw->W2S(clipPos, scrEnd);
                            DrawSmoothBoneLine(scrStart, scrEnd, CFG::Color_SniperLine, false);
                        }
                        else {
                            scrStart = Vec3();
                            H::Draw->W2S(clipPos, scrStart);
                            DrawSmoothBoneLine(scrStart, scrEnd, CFG::Color_SniperLine, false);
                        }
                    }
                }
                // ESP Tracer (inspired by Seowned - draw tracers from crosshair to enemy players)
                if (CFG::ESP_Tracer && isEnemy) {
                    Vec3 head = GetHitboxPosition(pPlayer, 0);
                    Vec3 scrHead;
                    if (H::Draw->W2S(head, scrHead)) {
                        int sw, sh;
                        I::EngineClient->GetScreenSize(sw, sh);
                        Vec3 center(static_cast<float>(sw) / 2.0f, static_cast<float>(sh) / 2.0f, 0.0f);
                        DrawSmoothBoneLine(center, scrHead, CFG::Color_TracerLine, true);
                    }
                }
                Color_t uberColor = CFG::Color_UberBar;
                Color_t outlineColor = CFG::Color_UberOutline;
                if (CFG::ESP_Uber)
                {
                    if (pPlayer->m_iClass() == TF_CLASS_MEDIC)
                    {
                        if (auto pWeapon = pPlayer->GetWeaponFromSlot(1))
                        {
                            const CFont& smallFont = H::Fonts->Get(EFonts::ESP_SMALL);
                            H::Draw->String(
                                smallFont,
                                rightTextX,
                                rightTextY,
                                CFG::Color_UberText,
                                POS_DEFAULT,
                                "%d%%", static_cast<int>(pWeapon->As<C_WeaponMedigun>()->m_flChargeLevel() * 100.0f)
                            );
                            rightTextY += smallFont.m_nTall + 1;
                        }
                    }
                }
                if (CFG::ESP_UberBar)
                {
                    if (pPlayer->m_iClass() == TF_CLASS_MEDIC)
                    {
                        if (auto pWeapon = pPlayer->GetWeaponFromSlot(1))
                        {
                            auto pMedigun = pWeapon->As<C_WeaponMedigun>();
                            if (auto flCharge = pMedigun->m_flChargeLevel())
                            {
                                int nBarH = 2;
                                int nDrawY = static_cast<int>(top) + height + nBarH + 1;
                                float flFillW = Math::RemapValClamped(flCharge, 0.0f, 1.0f, 0.0f, static_cast<float>(width));
                                H::Draw->OutlinedRect(static_cast<int>(left) - 1, nDrawY - 1, static_cast<int>(flFillW) + 2, nBarH + 2, outlineColor);
                                H::Draw->Rect(static_cast<int>(left), nDrawY, static_cast<int>(flFillW), nBarH, uberColor);
                                if (pMedigun->m_iItemDefinitionIndex() == Medic_s_TheVaccinator)
                                {
                                    if (flCharge >= 0.25f)
                                        H::Draw->Rect(static_cast<int>(left) + static_cast<int>(static_cast<float>(width) * 0.25f) - 1, nDrawY, 2, nBarH, outlineColor);
                                    if (flCharge >= 0.5f)
                                        H::Draw->Rect(static_cast<int>(left) + static_cast<int>(static_cast<float>(width) * 0.5f) - 1, nDrawY, 2, nBarH, outlineColor);
                                    if (flCharge >= 0.75f)
                                        H::Draw->Rect(static_cast<int>(left) + static_cast<int>(static_cast<float>(width) * 0.75f) - 1, nDrawY, 2, nBarH, outlineColor);
                                }
                            }
                        }
                    }
                }
            }
        }
        if ((drawSkeleton || drawBacktrackSkeleton) && (isPlayer || isBuilding) && onScreen) // Draw skeleton only if on screen
        {
            auto pAnimating = pBase->As<C_BaseAnimating>();
            if (!pAnimating) continue;
            matrix3x4_t boneMatrix[128] = {};
            const int maxBones = 128;
            bool ok = pAnimating->SetupBones(boneMatrix, maxBones, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime);
            if (!ok) continue;
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
                Color_t boneColor = isBuilding ? clr : CFG::Color_Skeleton;
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
                            DrawOutlinedLine(it1->second, it2->second, boneColor, CFG::Color_ESP_Outline);
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
                        ok = pAnimating->SetupBones(
                            boneMatrix,
                            maxBones,
                            BONE_USED_BY_ANYTHING,
                            pRecord->SimulationTime
                        );
                        if (!ok) continue;
                        float alphaFactor = 1.0f - static_cast<float>(n) / static_cast<float>(totalRecords + 1);
                        Color_t fadedClr = CFG::Color_BacktrackSkeleton;
                        fadedClr.a = static_cast<unsigned char>(static_cast<float>(fadedClr.a) * alphaFactor);
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
                                        DrawOutlinedLine(it1->second, it2->second, boneColor, CFG::Color_ESP_Outline);
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
        DrawFOVCircle(CFG::Aimbot_FOV, CFG::Color_AimbotFOV);
        DrawFOVCircle(CFG::Aimbot_Projectile_FOV, CFG::Color_ProjFOV);
        DrawFOVCircle(CFG::Aimbot_Melee_FOV, CFG::Color_MeleeFOV);
    }
}
CESP gESP;