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
C_BaseEntity* CESP::RainEntity = nullptr;
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
void CESP::DrawBox(int left, int top, int w, int h, const Color_t& clr)
{
    if (w <= 0 || h <= 0) return;
    H::Draw->Rect(left, top, w, h, clr);
}
void CESP::DrawBox2D(int left, int top, int w, int h, const Color_t& clr)
{
    Color_t black = Color_t(0, 0, 0, 255);
    H::Draw->Rect(left - 1, top - 1, w + 2, h + 2, black);
    H::Draw->Rect(left, top, w, h, clr);
    H::Draw->Rect(left + 1, top + 1, w - 2, h - 2, black);
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
    H::Draw->Line(left - 1, top - 1, left + cornerLen, top - 1, black);
    H::Draw->Line(left - 1, top - 1, left - 1, top + cornerLen, black);
    H::Draw->Line(left + 1, top + 1, left + cornerLen, top + 1, black);
    H::Draw->Line(left + 1, top + 1, left + 1, top + cornerLen, black);
    H::Draw->Line(left + w - cornerLen, top - 1, left + w + 1, top - 1, black);
    H::Draw->Line(left + w + 1, top - 1, left + w + 1, top + cornerLen, black);
    H::Draw->Line(left + w - cornerLen, top + 1, left + w - 1, top + 1, black);
    H::Draw->Line(left + w - 1, top + 1, left + w - 1, top + cornerLen, black);
    H::Draw->Line(left - 1, top + h - cornerLen, left - 1, top + h + 1, black);
    H::Draw->Line(left - 1, top + h + 1, left + cornerLen, top + h + 1, black);
    H::Draw->Line(left + 1, top + h - cornerLen, left + 1, top + h - 1, black);
    H::Draw->Line(left + 1, top + h - 1, left + cornerLen, top + h - 1, black);
    H::Draw->Line(left + w - cornerLen, top + h + 1, left + w + 1, top + h + 1, black);
    H::Draw->Line(left + w + 1, top + h - cornerLen, left + w + 1, top + h + 1, black);
    H::Draw->Line(left + w - cornerLen, top + h - 1, left + w - 1, top + h - 1, black);
    H::Draw->Line(left + w - 1, top + h - cornerLen, left + w - 1, top + h - 1, black);
    H::Draw->Line(left, top, left + cornerLen, top, clr);
    H::Draw->Line(left, top, left, top + cornerLen, clr);
    H::Draw->Line(left + w - cornerLen, top, left + w, top, clr);
    H::Draw->Line(left + w, top, left + w, top + cornerLen, clr);
    H::Draw->Line(left, top + h - cornerLen, left, top + h, clr);
    H::Draw->Line(left, top + h, left + cornerLen, top + h, clr);
    H::Draw->Line(left + w - cornerLen, top + h, left + w, top + h, clr);
    H::Draw->Line(left + w, top + h - cornerLen, left + w, top + h, clr);
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
void CESP::DrawThinLine(const Vec3& a, const Vec3& b, const Color_t& clr)
{
    H::Draw->Line(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y), clr);
}
void CESP::DrawOutlinedLine(const Vec3& a, const Vec3& b, const Color_t& clr, const Color_t& outlineClr)
{
    H::Draw->Line(static_cast<int>(a.x) - 1, static_cast<int>(a.y) - 1, static_cast<int>(b.x) - 1, static_cast<int>(b.y) - 1, outlineClr);
    H::Draw->Line(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y), clr);
    H::Draw->Line(static_cast<int>(a.x) + 1, static_cast<int>(a.y) + 1, static_cast<int>(b.x) + 1, static_cast<int>(b.y) + 1, outlineClr);
}
void CESP::DrawSmoothBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr, bool useAA)
{
    // Implementação simples sem AA por enquanto; adicione anti-aliasing se necessário
    H::Draw->Line(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y), clr);
}
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
    const float arrow_length = 25.0f;
    const float arrow_width = 22.0f;
    Vec3 dir = Vec3(x2 - x1, y2 - y1, 0.0f);
    float dirLen = dir.Length();
    if (dirLen <= 0.0001f) return;
    Vec3 unitDir = dir / dirLen;
    Vec3 tip = Vec3(x2, y2, 0.0f);
    Vec3 base = tip - unitDir * arrow_length;
    Vec3 perp = Vec3(-unitDir.y, unitDir.x, 0.0f);
    Vec3 left = base + perp * (arrow_width * 0.5f);
    Vec3 right = base - perp * (arrow_width * 0.5f);
    const float center_x = sw * 0.5f;
    const float center_y = sh * 0.5f;
    const Color_t outlineColor = Color_t(0, 0, 0, 255);
    Color_t shapeColor = Clr;
    int style = CFG::ESP_Offscreen_Style;
    bool filled = CFG::ESP_Offscreen_Filled;
    if (style == 0) {
        Vertex_t triangle[3];
        triangle[0].Init(Vector2D(center_x + tip.x, center_y + tip.y));
        triangle[1].Init(Vector2D(center_x + left.x, center_y + left.y));
        triangle[2].Init(Vector2D(center_x + right.x, center_y + right.y));
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
    else if (style == 1) {
        const float circle_radius = 12.0f;
        Vec3 circle_center = (tip + left + right) / 3.0f;
        int cx = static_cast<int>(center_x + circle_center.x);
        int cy = static_cast<int>(center_y + circle_center.y);
        if (filled) {
            H::Draw->CircleFilled(cx, cy, circle_radius, 32, shapeColor);
        }
        else {
            H::Draw->Circle(cx, cy, circle_radius, 32, shapeColor);
        }
    }
    else if (style == 2) {
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
    H::Draw->Circle(centerX, centerY, radius, 64, color);
}
void CESP::CustomFOV(CViewSetup* pSetup)
{
    if (!pSetup) return;
    auto pLocal = H::Entities->GetLocal();
    if (!pLocal) return;
    float fov = pSetup->fov;
    if (CFG::Visuals_CustomFov_Enable) fov = CFG::Visuals_CustomFov_Amount;
    if (InCond(pLocal, 1)) {
        if (CFG::Visuals_RemoveScopedZoom) fov = 90.0f;
    }
    pSetup->fov = fov;
}
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
    auto pSet = pHDR->pHitboxSet(0);
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

// ============================================================================
// ANIMAÇÃO DE BARRA DE VIDA
// ============================================================================

float CESP::GetAnimatedHealthValue(int entIndex, int currentHealth, int maxHealth)
{
    float currentTime = I::GlobalVars->realtime;
    float deltaTime = I::GlobalVars->frametime;

    // Se não existe animação para esta entidade, criar
    if (m_AnimatedHealth.find(entIndex) == m_AnimatedHealth.end())
    {
        m_AnimatedHealth[entIndex] = 0.0f;
        m_InitialAppearTime[entIndex] = currentTime;
    }

    float& animHealth = m_AnimatedHealth[entIndex];
    float targetHealth = static_cast<float>(currentHealth);

    // Verificar se é primeira vez aparecendo (animação de crescimento inicial)
    float timeSinceAppear = currentTime - m_InitialAppearTime[entIndex];
    if (timeSinceAppear < 1.0f) // 1 segundo para aparecer
    {
        // Crescimento de 0 até a vida atual
        float progress = timeSinceAppear; // 0.0 a 1.0
        animHealth = targetHealth * progress;
        return animHealth;
    }

    // Animação normal de mudança de vida
    float difference = targetHealth - animHealth;

    if (fabsf(difference) < 0.5f)
    {
        animHealth = targetHealth;
        return animHealth;
    }

    // Velocidade de animação
    float speed = 150.0f; // HP por segundo
    float maxChange = speed * deltaTime;

    if (fabsf(difference) <= maxChange)
    {
        animHealth = targetHealth;
    }
    else
    {
        if (difference > 0.0f)
            animHealth += maxChange; // Subindo (cura)
        else
            animHealth -= maxChange; // Descendo (dano)
    }

    return animHealth;
}

Color_t CESP::GetHealthBarColor(int health, int maxHealth)
{
    if (!CFG::ESP_HealthBarGradient)
    {
        // Retornar cor única (verde por padrão)
        return CFG::ESP_HealthBarColor;
    }

    // Sistema de gradiente com 3 cores
    float ratio = static_cast<float>(health) / static_cast<float>(maxHealth);
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    Color_t resultColor;

    if (ratio <= 0.5f)
    {
        // Interpolar entre Low (0%) e Mid (50%)
        float t = ratio * 2.0f; // 0.0 a 1.0
        resultColor = ColorLerp(CFG::ESP_HealthBarGradientLow, CFG::ESP_HealthBarGradientMid, t);
    }
    else
    {
        // Interpolar entre Mid (50%) e High (100%)
        float t = (ratio - 0.5f) * 2.0f; // 0.0 a 1.0
        resultColor = ColorLerp(CFG::ESP_HealthBarGradientMid, CFG::ESP_HealthBarGradientHigh, t);
    }

    return resultColor;
}

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
    const int maxClients = I::EngineClient->GetMaxClients();
    const int highestIndex = I::ClientEntityList->GetHighestEntityIndex();
    const int end = std::max(maxClients, highestIndex);
    int localTeam = -1;
    pLocal->IsInValidTeam(&localTeam);
    int localIndex = I::EngineClient->GetLocalPlayer();
    auto pResource = GetTFPlayerResource();
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
        bool isEnemy = !isTeammate && team > 1;
        bool isLocal = (i == localIndex);
        bool drawESP = false;
        bool isPlayer = false;
        bool isBuilding = false;
        bool isPickup = false;
        bool isFlag = false;
        std::string name = "";
        int health = 0;
        int max_health = 100;
        Color_t clr = Color_t(255, 255, 255, 255);
        C_TFPlayer* pPlayer = nullptr;
        if (strcmp(networkName, "CTFPlayer") == 0)
        {
            pPlayer = static_cast<C_TFPlayer*>(pBase);
            if (!pPlayer) continue;
            if (pPlayer->m_lifeState() != LIFE_ALIVE) continue;
            isPlayer = true;
            bool isCloaked = InCond(pPlayer, 4);
            if (isCloaked && CFG::ESP_HideCloaked) continue;
            clr = isLocal ? CFG::Color_Local : (team == TF_TEAM_RED ? CFG::Color_TeamRed : CFG::Color_TeamBlue);
            drawESP = (isLocal ? CFG::ESP_LocalPlayer : true) && !(CFG::ESP_Team && isTeammate && !isLocal) && !(isCloaked && CFG::ESP_HideCloaked);
            if (isLocal) {
                if (!CFG::Misc_ThirdPerson_Enable) {
                    drawESP = false;
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
        }
        else if (strcmp(networkName, "CObjectSentrygun") == 0 || strcmp(networkName, "CObjectDispenser") == 0 || strcmp(networkName, "CObjectTeleporter") == 0)
        {
            if (!CFG::ESP_Build) continue;
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
        }
        else if ((strcmp(networkName, "CTFAmmoPack") == 0 ||
            strstr(networkName, "item_healthkit_") ||
            strstr(networkName, "item_ammopack_")) ||
            (strcmp(networkName, "CBaseAnimating") == 0 &&
                (H::Entities->IsHealthPack(pBase) || H::Entities->IsAmmoPack(pBase))))
        {
            if (strstr(networkName, "debris") && !strstr(networkName, "item_")) continue;
            if (!CFG::ESP_Pickups) continue;
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
        }
        else if (strcmp(networkName, "CCaptureFlag") == 0 ||
            strcmp(networkName, "CTFItemTeamFlag") == 0 ||
            strcmp(networkName, "CItemTeamFlag") == 0 ||
            strcmp(networkName, "item_teamflag") == 0 ||
            strcmp(networkName, "CTeamControlPoint") == 0 ||
            strcmp(networkName, "CFuncTrackTrain") == 0)
        {
            if (CFG::ESP_Team && isTeammate) continue;
            isFlag = true;
            if (strstr(networkName, "flag") || strstr(networkName, "Flag")) name = "Intel";
            else if (strcmp(networkName, "CTeamControlPoint") == 0) name = "Control Point";
            else name = "Payload Cart";
            drawESP = CFG::ESP_CaptureFlag;
            clr = CFG::Color_Flag;
        }
        else continue;
        Vec3 origin = pBase->GetAbsOrigin();
        float dist = (pLocal->GetShootPos() - origin).Length() / 39.37f;
        Vec3 mins = pBase->m_vecMins();
        Vec3 maxs = pBase->m_vecMaxs();
        if (!IsFiniteVec(mins) || !IsFiniteVec(maxs) || !IsFiniteVec(origin)) continue;
        if (mins.Length() < 0.1f || maxs.Length() < 0.1f) {
            if (auto pAnim = pBase->As<C_BaseAnimating>()) {
                auto pModel = pAnim->GetModel();
                if (pModel) {
                    auto pStudio = I::ModelInfoClient->GetStudiomodel(pModel);
                    if (pStudio) {
                        mins = pStudio->hull_min;
                        maxs = pStudio->hull_max;
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
        if (!allProjected) continue;
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
                if (draw_box && drawESP) {
                    int boxLeft = static_cast<int>(std::round(left));
                    int boxTop = static_cast<int>(std::round(top));
                    int boxW = width;
                    int boxH = height;
                    bool useAA = (dist < 1500.0f);
                    switch (CFG::ESP_BoxType) {
                    case 0: DrawBox2D(boxLeft, boxTop, boxW, boxH, clr); break;
                    case 1: DrawBox3D(screenPts, clr, useAA); break;
                    case 2: DrawBoxCorner(boxLeft, boxTop, boxW, boxH, clr); break;
                    default: break;
                    }
                }
                if (draw_name && !name.empty())
                {
                    const CFont& fontObj = H::Fonts->Get(EFonts::ESP);
                    HFont font = fontObj.m_dwFont;
                    Color_t nameClr = isBuilding ? CFG::Color_BuildingName : CFG::Color_Name;
                    H::Draw->Text(static_cast<int>((left + right) / 2.0f), static_cast<int>(top) - 15, font, nameClr, ALIGN_CENTER_H, name.c_str());
                }
                if (CFG::ESP_Health && max_health > 0 && (isPlayer || isBuilding))
                {
                    // Obter vida animada
                    float animatedHealth = GetAnimatedHealthValue(i, health, max_health);

                    int healthType = CFG::ESP_HealthType;
                    int position = CFG::ESP_HealthBarPosition; // 0=Left, 1=Right, 2=Top, 3=Bottom
                    bool hasBar = (healthType == 0 || healthType == 2);

                    // Calcular posições baseadas na escolha
                    int barX, barY, barW, barH;
                    bool isVertical = (position == 0 || position == 1); // Left ou Right = vertical

                    if (isVertical)
                    {
                        // Barra vertical (Left ou Right)
                        barW = 4;
                        barH = height + 2;
                        barY = static_cast<int>(std::round(top)) - 1;

                        if (position == 0) // Left
                            barX = static_cast<int>(std::round(left)) - 6;
                        else // Right
                            barX = static_cast<int>(std::round(right)) + 2;
                    }
                    else
                    {
                        // Barra horizontal (Top ou Bottom)
                        barW = width + 2;
                        barH = 4;
                        barX = static_cast<int>(std::round(left)) - 1;

                        if (position == 2) // Top
                            barY = static_cast<int>(std::round(top)) - 6;
                        else // Bottom
                            barY = static_cast<int>(std::round(bottom)) + 2;
                    }

                    if (hasBar)
                    {
                        // Desenhar fundo
                        H::Draw->RectFilled(barX, barY, barW, barH, CFG::Color_HealthBarBG);

                        if (animatedHealth > 0)
                        {
                            // Calcular proporção
                            float ratio = animatedHealth / static_cast<float>(max_health);
                            ratio = std::min(1.0f, ratio);

                            // Calcular tamanho preenchido baseado na orientação
                            int fillW, fillH, fillX, fillY;

                            if (isVertical)
                            {
                                // Barra vertical - cresce de baixo para cima
                                fillW = 2;
                                fillH = static_cast<int>(std::round((barH - 2) * ratio));
                                fillX = barX + 1;
                                fillY = barY + (barH - 1) - fillH;
                            }
                            else
                            {
                                // Barra horizontal - cresce da esquerda para direita
                                fillW = static_cast<int>(std::round((barW - 2) * ratio));
                                fillH = 2;
                                fillX = barX + 1;
                                fillY = barY + 1;
                            }

                            // ✅ NOVO SISTEMA DE COR
                            Color_t healthColor;

                            // Verificar overheal
                            if (health > max_health)
                            {
                                healthColor = CFG::Color_Overheal;
                            }
                            else
                            {
                                // Usar o sistema de gradiente ou cor única
                                healthColor = GetHealthBarColor(health, max_health);
                            }

                            // Desenhar barra preenchida
                            if ((isVertical && fillH > 0) || (!isVertical && fillW > 0))
                            {
                                H::Draw->RectFilled(fillX, fillY, fillW, fillH, healthColor);
                            }
                        }
                    }

                    // Desenhar texto de vida (se habilitado)
                    if (healthType == 1 || healthType == 2)
                    {
                        const CFont& fontObj = H::Fonts->Get(EFonts::ESP);
                        HFont font = fontObj.m_dwFont;
                        std::string healthStr = std::to_string(health);

                        int textW, textH;
                        H::Draw->GetTextSize(font, healthStr.c_str(), textW, textH);

                        int textX, textY;

                        // Posicionar texto baseado na posição da barra
                        switch (position)
                        {
                        case 0: // Left
                            if (hasBar)
                                textX = barX - textW - 2;
                            else
                                textX = static_cast<int>(left) - textW - 2;
                            textY = static_cast<int>(top);
                            break;

                        case 1: // Right
                            if (hasBar)
                                textX = barX + barW + 2;
                            else
                                textX = static_cast<int>(right) + 5;
                            textY = static_cast<int>(top);
                            break;

                        case 2: // Top
                            textX = static_cast<int>((left + right) / 2.0f) - (textW / 2);
                            if (hasBar)
                                textY = barY - textH - 2;
                            else
                                textY = static_cast<int>(top) - textH - 2;
                            break;

                        case 3: // Bottom
                            textX = static_cast<int>((left + right) / 2.0f) - (textW / 2);
                            if (hasBar)
                                textY = barY + barH + 2;
                            else
                                textY = static_cast<int>(bottom) + 5;
                            break;

                        default:
                            textX = static_cast<int>(left) - textW - 2;
                            textY = static_cast<int>(top);
                            break;
                        }

                        H::Draw->Text(textX, textY, font, CFG::Color_HealthText, ALIGN_DEFAULT, healthStr.c_str());
                    }
                }
            }
            if (isPlayer && drawESP) {
                int playerClass = pPlayer->m_iClass();
                const CFont& fontObj = H::Fonts->Get(EFonts::ESP);
                HFont font = fontObj.m_dwFont;
                int rightTextX = static_cast<int>(right) + 5;
                int rightTextY = static_cast<int>(top);
                if (CFG::ESP_Conds) {
                    std::vector<std::string> conds;
                    if (InCond(pPlayer, 4)) conds.push_back("Cloaked");
                    if (InCond(pPlayer, 3)) conds.push_back("Disguised");
                    if (InCond(pPlayer, 5)) conds.push_back("Uber");
                    if (InCond(pPlayer, 22)) conds.push_back("Burning");
                    if (InCond(pPlayer, 7)) conds.push_back("Taunt");
                    for (const auto& c : conds) {
                        H::Draw->Text(rightTextX, rightTextY, font, CFG::Color_CondsText, ALIGN_DEFAULT, c.c_str());
                        rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                    }
                }
                if (CFG::ESP_Buffs || CFG::ESP_Debuffs) {
                    std::vector<std::string> buffs, debuffs;
                    if (InCond(pPlayer, 5)) buffs.push_back("Uber");
                    if (InCond(pPlayer, 11)) buffs.push_back("Kritz");
                    if (InCond(pPlayer, 16)) buffs.push_back("Banner");
                    if (InCond(pPlayer, 18)) buffs.push_back("Charging");
                    if (InCond(pPlayer, 22)) debuffs.push_back("Burning");
                    if (InCond(pPlayer, 25)) debuffs.push_back("Bleeding");
                    if (InCond(pPlayer, 14)) debuffs.push_back("Bonked");
                    if (InCond(pPlayer, 21)) debuffs.push_back("Marked");
                    if (InCond(pPlayer, 28)) debuffs.push_back("Milk");
                    if (CFG::ESP_Buffs) {
                        for (const auto& b : buffs) {
                            H::Draw->Text(rightTextX, rightTextY, font, Color_t(0, 255, 0, 255), ALIGN_DEFAULT, b.c_str());
                            rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                        }
                    }
                    if (CFG::ESP_Debuffs) {
                        for (const auto& d : debuffs) {
                            H::Draw->Text(rightTextX, rightTextY, font, Color_t(255, 0, 0, 255), ALIGN_DEFAULT, d.c_str());
                            rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                        }
                    }
                }
                if (CFG::ESP_DistanceEnemy && isEnemy) {
                    std::string distStr = std::to_string(static_cast<int>(dist)) + " m";
                    if (CFG::ESP_DistancePosition == 0) {
                        H::Draw->Text(rightTextX, rightTextY, font, Color_t(255, 255, 255, 255), ALIGN_DEFAULT, distStr.c_str());
                        rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                    }
                    else {
                        H::Draw->Text(static_cast<int>((left + right) / 2.0f), static_cast<int>(bottom) + 5, font, Color_t(255, 255, 255, 255), ALIGN_CENTER_H, distStr.c_str());
                    }
                }
                if (CFG::ESP_Ping) {
                    static int pingOffset = NetVars::GetNetVar("CPlayerResource", "m_iPing");
                    int ping = pResource ? *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + pingOffset + i * sizeof(int)) : 0;
                    std::string pingStr = std::to_string(ping) + " ms";
                    H::Draw->Text(rightTextX, rightTextY, font, Color_t(255, 255, 255, 255), ALIGN_DEFAULT, pingStr.c_str());
                    rightTextY += H::Fonts->GetFontHeight(EFonts::ESP) + 1;
                }
                if (CFG::ESP_SniperLines && isEnemy && playerClass == 2) {
                    Vec3 eyePos = pPlayer->GetShootPos();
                    Vec3 ang = pPlayer->GetEyeAngles();
                    Vec3 fwd;
                    Math::AngleVectors(ang, &fwd);
                    Vec3 end = eyePos + fwd * 16384.0f;
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
                            const CFont& smallFontObj = H::Fonts->Get(EFonts::ESP_SMALL);
                            HFont smallFont = smallFontObj.m_dwFont;
                            H::Draw->TextF(
                                rightTextX,
                                rightTextY,
                                smallFont,
                                CFG::Color_UberText,
                                ALIGN_DEFAULT,
                                "%d%%", static_cast<int>(pWeapon->As<C_WeaponMedigun>()->m_flChargeLevel() * 100.0f)
                            );
                            rightTextY += H::Fonts->GetFontHeight(EFonts::ESP_SMALL) + 1;
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
                                H::Draw->Rect(static_cast<int>(left) - 1, nDrawY - 1, static_cast<int>(flFillW) + 2, nBarH + 2, outlineColor);
                                H::Draw->RectFilled(static_cast<int>(left), nDrawY, static_cast<int>(flFillW), nBarH, uberColor);
                                if (pMedigun->m_iItemDefinitionIndex() == Medic_s_TheVaccinator)
                                {
                                    if (flCharge >= 0.25f)
                                        H::Draw->RectFilled(static_cast<int>(left) + static_cast<int>(static_cast<float>(width) * 0.25f) - 1, nDrawY, 2, nBarH, outlineColor);
                                    if (flCharge >= 0.5f)
                                        H::Draw->RectFilled(static_cast<int>(left) + static_cast<int>(static_cast<float>(width) * 0.5f) - 1, nDrawY, 2, nBarH, outlineColor);
                                    if (flCharge >= 0.75f)
                                        H::Draw->RectFilled(static_cast<int>(left) + static_cast<int>(static_cast<float>(width) * 0.75f) - 1, nDrawY, 2, nBarH, outlineColor);
                                }
                            }
                        }
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