#include "ESP.h"
#include "../src/SDK/TF2/interface.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include "CFG.h"
#include "../src/SDK/SDK.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstring>
#include <limits>
#include <cstdint>
#include <unordered_map>
#include <string>

#define PI 3.14159265358979323846f
#define DEG2RAD(deg) ((deg) * PI / 180.0f)

struct BoneMatrixes_t
{
    float BoneMatrix[128][3][4];
};

static bool IsFiniteVec(const Vec3& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

static bool InCond(C_TFPlayer* pPlayer, int cond)
{
    if (!pPlayer) return false;
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

static Vec3 GetHitboxPosition(C_TFPlayer* pPlayer, int iHitbox)
{
    matrix3x4_t boneMatrix[128];
    if (!pPlayer->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime))
        return Vec3();

    auto pModel = pPlayer->GetModel();
    if (!pModel)
        return Vec3();

    auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR)
        return Vec3();

    auto pSet = pHDR->pHitboxSet(0);
    if (!pSet)
        return Vec3();

    auto pBox = pSet->pHitbox(iHitbox);
    if (!pBox)
        return Vec3();

    Vec3 vMin, vMax;
    Math::VectorTransform(pBox->bbmin, boneMatrix[pBox->bone], vMin);
    Math::VectorTransform(pBox->bbmax, boneMatrix[pBox->bone], vMax);
    return (vMin + vMax) * 0.5f;
}

void CESP::StoreBoneMatrix(C_TFPlayer* pPlayer)
{
    if (!pPlayer || pPlayer->IsDormant())
        return;

    matrix3x4_t aBones[128];
    if (!pPlayer->SetupBones(aBones, 128, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime))
        return;

    BoneMatrixes_t boneMatrix;
    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                boneMatrix.BoneMatrix[i][j][k] = aBones[i][j][k];
            }
        }
    }

    m_mBones[pPlayer] = boneMatrix;
}

void CESP::StoreShotSnapshot(C_TFPlayer* pPlayer)
{
    if (!pPlayer || pPlayer->IsDormant()) return;

    matrix3x4_t bones[128];
    if (!pPlayer->SetupBones(bones, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime))
        return;

    BoneMatrixes_t bm{};
    for (int i = 0; i < 128; ++i)
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 4; ++c)
                bm.BoneMatrix[i][r][c] = bones[i][r][c];

    m_mBonesOnShot[pPlayer] = { bm, I::GlobalVars->realtime };

    std::vector<Vec3> hitboxPoints;
    auto pModel = pPlayer->GetModel();
    if (pModel) {
        auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
        if (pHDR) {
            auto pSet = pHDR->pHitboxSet(pPlayer->m_nHitboxSet());
            if (pSet) {
                for (int i = 0; i < pSet->numhitboxes; ++i) {
                    auto pBox = pSet->pHitbox(i);
                    if (!pBox) continue;

                    Vec3 boxExtents = pBox->bbmax - pBox->bbmin;
                    if (boxExtents.Length() < 0.001f) continue;
                    if (pBox->bone < 0 || pBox->bone >= 128) continue;

                    Vec3 localCorners[8] = {
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
                    for (int j = 0; j < 8; ++j) {
                        Math::VectorTransform(localCorners[j], bones[pBox->bone], worldCorners[j]);
                    }

                    Vec3 center = Vec3(0, 0, 0);
                    for (int j = 0; j < 8; ++j) center += worldCorners[j];
                    center /= 8.0f;
                    hitboxPoints.push_back(center);

                    for (int j = 0; j < 8; ++j) {
                        hitboxPoints.push_back(worldCorners[j]);
                    }
                }
            }
        }
    }

    m_mBoneHitboxesOnShot[pPlayer] = { hitboxPoints, I::GlobalVars->realtime };
}

void CESP::StoreHitSnapshot(C_TFPlayer* pPlayer, const Vec3& hitPos)
{
    if (!pPlayer || pPlayer->IsDormant()) return;

    matrix3x4_t bones[128];
    if (!pPlayer->SetupBones(bones, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime))
        return;

    BoneMatrixes_t bm{};
    for (int i = 0; i < 128; ++i)
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 4; ++c)
                bm.BoneMatrix[i][r][c] = bones[i][r][c];

    m_mBonesOnHit[pPlayer] = { bm, I::GlobalVars->realtime };

    std::vector<Vec3> hitboxPoints;
    auto pModel = pPlayer->GetModel();
    if (pModel) {
        auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
        if (pHDR) {
            auto pSet = pHDR->pHitboxSet(pPlayer->m_nHitboxSet());
            if (pSet) {
                for (int i = 0; i < pSet->numhitboxes; ++i) {
                    auto pBox = pSet->pHitbox(i);
                    if (!pBox) continue;

                    Vec3 boxExtents = pBox->bbmax - pBox->bbmin;
                    if (boxExtents.Length() < 0.001f) continue;
                    if (pBox->bone < 0 || pBox->bone >= 128) continue;

                    Vec3 localCorners[8] = {
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
                    for (int j = 0; j < 8; ++j) {
                        Math::VectorTransform(localCorners[j], bones[pBox->bone], worldCorners[j]);
                    }

                    Vec3 center = Vec3(0, 0, 0);
                    for (int j = 0; j < 8; ++j) center += worldCorners[j];
                    center /= 8.0f;
                    hitboxPoints.push_back(center);

                    for (int j = 0; j < 8; ++j) {
                        hitboxPoints.push_back(worldCorners[j]);
                    }
                }
            }
        }
    }

    m_mBoneHitboxesOnHit[pPlayer] = { hitboxPoints, I::GlobalVars->realtime };
    m_mHitPosOnHit[pPlayer] = { hitPos, I::GlobalVars->realtime };
}

Vec3 CESP::GetBonePosition(C_TFPlayer* pPlayer, int iBone, const BoneMatrixes_t* pBoneMatrix)
{
    if (!pPlayer || iBone < 0 || iBone >= 128)
        return Vec3();

    const BoneMatrixes_t* usedMatrix = nullptr;

    if (pBoneMatrix) {
        usedMatrix = pBoneMatrix;
    }
    else {
        auto it = m_mBones.find(pPlayer);
        if (it != m_mBones.end())
            usedMatrix = &it->second;
        else
            return Vec3(); // Retorna zero se não encontrar
    }

    Vec3 vBone;
    vBone.x = usedMatrix->BoneMatrix[iBone][0][3];
    vBone.y = usedMatrix->BoneMatrix[iBone][1][3];
    vBone.z = usedMatrix->BoneMatrix[iBone][2][3];

    return vBone;
}

void CESP::StoreBoneHitboxes(C_TFPlayer* pPlayer)
{
    if (!pPlayer || pPlayer->IsDormant())
        return;

    auto pModel = pPlayer->GetModel();
    if (!pModel)
        return;

    auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR)
        return;

    auto pSet = pHDR->pHitboxSet(pPlayer->m_nHitboxSet());
    if (!pSet)
        return;

    matrix3x4_t aBones[128];
    if (!pPlayer->SetupBones(aBones, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime))
        return;

    std::vector<Vec3> hitboxPoints;

    for (int i = 0; i < pSet->numhitboxes; i++)
    {
        auto pBox = pSet->pHitbox(i);
        if (!pBox)
            continue;

        Vec3 ext = pBox->bbmax - pBox->bbmin;
        if (ext.Length() < 0.001f) continue;
        if (pBox->bone < 0 || pBox->bone >= 128) continue;

        Vec3 localCorners[8] = {
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
        for (int j = 0; j < 8; ++j) {
            Math::VectorTransform(localCorners[j], aBones[pBox->bone], worldCorners[j]);
        }

        Vec3 center = Vec3(0, 0, 0);
        for (int j = 0; j < 8; ++j) center += worldCorners[j];
        center /= 8.0f;
        hitboxPoints.push_back(center);

        for (int j = 0; j < 8; ++j) {
            hitboxPoints.push_back(worldCorners[j]);
        }
    }

    m_mBoneHitboxes[pPlayer] = hitboxPoints;
}

static bool AreBonePositionsValid(const Vec3& bone1, const Vec3& bone2, float minDist = 2.0f, float maxDist = 500.0f)
{
    // Verificar se os ossos não estão zerados
    if (bone1.Length() < 0.1f || bone2.Length() < 0.1f)
        return false;

    // Verificar se os valores são finitos
    if (!IsFiniteVec(bone1) || !IsFiniteVec(bone2))
        return false;

    // Calcular distância entre os ossos
    float dist = (bone1 - bone2).Length();

    // A distância deve estar dentro de um intervalo razoável
    // (não muito perto, não muito longe)
    if (dist < minDist || dist > maxDist)
        return false;

    return true;
}

void CESP::DrawSkeleton(C_TFPlayer* pPlayer, const Color_t& clr)
{
    if (!pPlayer || pPlayer->IsDormant())
        return;

    // Tentar pegar matrizes das bones (hitbox é mais estável para ESP)
    matrix3x4_t aBones[128];
    bool gotBones = pPlayer->SetupBones(aBones, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime);

    // fallback para matriz armazenada
    if (!gotBones) {
        auto it = m_mBones.find(pPlayer);
        if (it == m_mBones.end())
            return;
        for (int b = 0; b < 128; ++b)
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 4; ++c)
                    aBones[b][r][c] = it->second.BoneMatrix[b][r][c];
        gotBones = true;
    }

    if (!gotBones)
        return;

    auto pModel = pPlayer->GetModel();
    if (!pModel) return;

    auto pStudio = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pStudio) return;

    const int numbones = pStudio->numbones;
    if (numbones <= 0) return;

    // Função utilitária para extrair posição da matriz
    auto BonePosFromMatrix = [&](int idx) -> Vec3 {
        if (idx < 0 || idx >= 128) return Vec3();
        Vec3 v;
        v.x = aBones[idx][0][3];
        v.y = aBones[idx][1][3];
        v.z = aBones[idx][2][3];
        return v;
    };

    // Nomes que normalmente geram linhas desnecessárias (dedos, attachments, etc.)
    auto IsUnwantedBoneName = [&](const char* name) -> bool {
        if (!name) return true;
        // palavras-chave comuns a excluir
        return strstr(name, "finger") || strstr(name, "thumb") ||
               strstr(name, "middle") || strstr(name, "ring") ||
               strstr(name, "pinky")  || strstr(name, "toe") ||
               strstr(name, "hat")   || strstr(name, "cap") ||
               strstr(name, "attach")|| strstr(name, "weapon")||
               strstr(name, "prop")  || strstr(name, "eyelid")||
               strstr(name, "eye")   || strstr(name, "jaw") ||
               strstr(name, "tongue")|| strstr(name, "lbrow")||
               strstr(name, "rbrow") || strstr(name, "lips");
    };

    // Percorre todos os bones e desenha linha bone <-> parent
    for (int i = 0; i < numbones; ++i)
    {
        mstudiobone_t* pBone = pStudio->pBone(i);
        if (!pBone)
            continue;

        int parent = pBone->parent;
        if (parent < 0 || parent >= numbones)
            continue;

        // filtrar nomes indesejados para evitar linhas soltas
        const char* boneName = pBone->pszName();
        if (IsUnwantedBoneName(boneName))
            continue;

        // obter posições a partir das matrizes (ou retorno vazio)
        Vec3 posBone = BonePosFromMatrix(i);
        Vec3 posParent = BonePosFromMatrix(parent);

        // validações
        if (!IsFiniteVec(posBone) || !IsFiniteVec(posParent))
            continue;

        if (!AreBonePositionsValid(posBone, posParent, 1.0f, 300.0f))
            continue;

        // projetar para tela
        Vec3 scrA, scrB;
        if (!H::Draw->W2S(posBone, scrA) || !H::Draw->W2S(posParent, scrB))
            continue;

        if (!IsFiniteVec(scrA) || !IsFiniteVec(scrB))
            continue;

        DrawOutlinedLine(scrA, scrB, clr, CFG::Color_ESP_Outline);
    }
}

void CESP::SetAimbotAimPoint(C_TFPlayer* pPlayer, const Vec3& point)
{
    if (!pPlayer) return;
    m_mAimbotAimPoints[pPlayer] = { point, I::GlobalVars->realtime };
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
    H::Draw->Line(left + w - cornerLen, top - 1, left + w + 1, top - 1, black);
    H::Draw->Line(left + w + 1, top - 1, left + w + 1, top + cornerLen, black);
    H::Draw->Line(left - 1, top + h - cornerLen, left - 1, top + h + 1, black);
    H::Draw->Line(left - 1, top + h + 1, left + cornerLen, top + h + 1, black);
    H::Draw->Line(left + w - cornerLen, top + h + 1, left + w + 1, top + h + 1, black);
    H::Draw->Line(left + w + 1, top + h - cornerLen, left + w + 1, top + h + 1, black);

    H::Draw->Line(left, top, left + cornerLen, top, clr);
    H::Draw->Line(left, top, left, top + cornerLen, clr);
    H::Draw->Line(left + w - cornerLen, top, left + w, top, clr);
    H::Draw->Line(left + w, top, left + w, top + cornerLen, clr);
    H::Draw->Line(left, top + h - cornerLen, left, top + h, clr);
    H::Draw->Line(left, top + h, left + cornerLen, top + h, clr);
    H::Draw->Line(left + w - cornerLen, top + h, left + w, top + h, clr);
    H::Draw->Line(left + w, top + h - cornerLen, left + w, top + h, clr);
}

void CESP::DrawImpactBox(const Vec3& worldPos, const Color_t& clr)
{
    if (!H::Draw) return;

    Vec3 mins(-4.0f, -4.0f, -4.0f);
    Vec3 maxs(4.0f, 4.0f, 4.0f);
    Vec3 points[8] = {
        { worldPos.x + mins.x, worldPos.y + mins.y, worldPos.z + mins.z },
        { worldPos.x + mins.x, worldPos.y + maxs.y, worldPos.z + mins.z },
        { worldPos.x + maxs.x, worldPos.y + maxs.y, worldPos.z + mins.z },
        { worldPos.x + maxs.x, worldPos.y + mins.y, worldPos.z + mins.z },
        { worldPos.x + maxs.x, worldPos.y + maxs.z, worldPos.z + maxs.z },
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
        {0,4},{1,5},{2,6},{3,5}
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
    H::Draw->Line(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y), clr);
}

void CESP::DrawProjectedHitboxWire(const Vec3 proj[8], const Color_t& clr, bool useAA)
{
    if (!H::Draw) return;

    for (int i = 0; i < 8; ++i) {
        if (!std::isfinite(proj[i].x) || !std::isfinite(proj[i].y))
            return;
    }

    const std::pair<int, int> edges[] = {
        {0,1},{1,2},{2,3},{3,0},
        {7,6},{6,4},{4,5},{5,7},
        {0,7},{1,6},{2,4},{3,5}
    };

    for (auto& e : edges) {
        const Vec3& A = proj[e.first];
        const Vec3& B = proj[e.second];
        DrawThinLine(A, B, clr);
    }
}

void CESP::PlayerArrow(C_TFPlayer* Player, Color_t Clr)
{
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

    Color_t shapeColor = Clr;
    int style = CFG::ESP_Offscreen_Style;
    bool filled = CFG::ESP_Offscreen_Filled;
    Color_t filledColor = CFG::ESP_OffscreenFilledColor;

    if (style == 0) {
        Vertex_t triangle[3];
        triangle[0].Init(Vector2D(center_x + tip.x, center_y + tip.y));
        triangle[1].Init(Vector2D(center_x + left.x, center_y + left.y));
        triangle[2].Init(Vector2D(center_x + right.x, center_y + right.y));

        if (filled) {
            H::Draw->Polygon(3, triangle, filledColor);
            H::Draw->Line(static_cast<int>(triangle[0].m_Position.x), static_cast<int>(triangle[0].m_Position.y),
                static_cast<int>(triangle[1].m_Position.x), static_cast<int>(triangle[1].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(triangle[1].m_Position.x), static_cast<int>(triangle[1].m_Position.y),
                static_cast<int>(triangle[2].m_Position.x), static_cast<int>(triangle[2].m_Position.y), shapeColor);
            H::Draw->Line(static_cast<int>(triangle[2].m_Position.x), static_cast<int>(triangle[2].m_Position.y),
                static_cast<int>(triangle[0].m_Position.x), static_cast<int>(triangle[0].m_Position.y), shapeColor);
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
            H::Draw->CircleFilled(cx, cy, circle_radius, 32, filledColor);
            H::Draw->Circle(cx, cy, circle_radius, 32, shapeColor);
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
            H::Draw->Polygon(4, quad, filledColor);
            for (int i = 0; i < 4; ++i) {
                int next = (i + 1) % 4;
                H::Draw->Line(static_cast<int>(quad[i].m_Position.x), static_cast<int>(quad[i].m_Position.y),
                    static_cast<int>(quad[next].m_Position.x), static_cast<int>(quad[next].m_Position.y), shapeColor);
            }
        }
        else {
            for (int i = 0; i < 4; ++i) {
                int next = (i + 1) % 4;
                H::Draw->Line(static_cast<int>(quad[i].m_Position.x), static_cast<int>(quad[i].m_Position.y),
                    static_cast<int>(quad[next].m_Position.x), static_cast<int>(quad[next].m_Position.y), shapeColor);
            }
        }
    }
}

void CESP::DrawFOVCircle(float fov, const Color_t& color)
{
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
    if (CFG::Visuals_CustomFov_Enable)
        fov = CFG::Visuals_CustomFov_Amount;

    if (InCond(pLocal, 1)) {
        if (CFG::Visuals_RemoveScopedZoom)
            fov = 90.0f;
    }

    pSetup->fov = fov;
}

float CESP::GetAnimatedHealthValue(int entIndex, int currentHealth, int maxHealth)
{
    float currentTime = I::GlobalVars->realtime;
    float deltaTime = I::GlobalVars->frametime;

    if (m_AnimatedHealth.find(entIndex) == m_AnimatedHealth.end()) {
        m_AnimatedHealth[entIndex] = 0.0f;
        m_InitialAppearTime[entIndex] = currentTime;
    }

    float& animHealth = m_AnimatedHealth[entIndex];
    float targetHealth = static_cast<float>(currentHealth);

    float timeSinceAppear = currentTime - m_InitialAppearTime[entIndex];
    if (timeSinceAppear < 1.0f) {
        float progress = timeSinceAppear;
        animHealth = targetHealth * progress;
        return animHealth;
    }

    float difference = targetHealth - animHealth;

    if (fabsf(difference) < 0.5f) {
        animHealth = targetHealth;
        return animHealth;
    }

    float speed = 150.0f;
    float maxChange = speed * deltaTime;

    if (fabsf(difference) <= maxChange) {
        animHealth = targetHealth;
    }
    else {
        if (difference > 0.0f)
            animHealth += maxChange;
        else
            animHealth -= maxChange;
    }

    return animHealth;
}

Color_t CESP::GetHealthBarColor(int health, int maxHealth)
{
    if (!CFG::ESP_HealthBarGradient)
        return CFG::ESP_HealthBarColor;

    float ratio = static_cast<float>(health) / static_cast<float>(maxHealth);
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    Color_t resultColor;

    if (ratio <= 0.5f) {
        float t = ratio * 2.0f;
        resultColor = ColorLerp(CFG::ESP_HealthBarGradientLow, CFG::ESP_HealthBarGradientMid, t);
    }
    else {
        float t = (ratio - 0.5f) * 2.0f;
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

            StoreBoneMatrix(pPlayer);
            StoreBoneHitboxes(pPlayer);

            clr = isLocal ? CFG::Color_Local : (team == TF_TEAM_RED ? CFG::Color_TeamRed : CFG::Color_TeamBlue);
            drawESP = (isLocal ? CFG::ESP_LocalPlayer : true) && !(CFG::ESP_Team && isTeammate && !isLocal) && !(isCloaked && CFG::ESP_HideCloaked);

            if (isLocal && !CFG::Misc_ThirdPerson_Enable)
                drawESP = false;

            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(i, &info))
                name = info.name ? info.name : "unknown";

            if (pResource) {
                static int healthOffset = NetVars::GetNetVar("CPlayerResource", "m_iHealth");
                static int maxHealthOffset = NetVars::GetNetVar("CTFPlayerResource", "m_iMaxHealth");
                health = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + healthOffset + i * sizeof(int));
                max_health = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + maxHealthOffset + i * sizeof(int));
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
            clr = CFG::ESP_BuildColor;

            if (strcmp(networkName, "CObjectSentrygun") == 0)
                name = "Sentry";
            else if (strcmp(networkName, "CObjectDispenser") == 0)
                name = "Dispenser";
            else
                name = "Teleporter";

            int level = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pBase) + NetVars::GetNetVar("CBaseObject", "m_iUpgradeLevel"));
            name += " Lvl " + std::to_string(level);
            health = static_cast<C_BaseObject*>(pBase)->m_iHealth();
            max_health = static_cast<C_BaseObject*>(pBase)->m_iMaxHealth();
            drawESP = CFG::ESP_Build && !(CFG::ESP_BuildOnlyEnemy && !isEnemy);
        }
        else if ((strcmp(networkName, "CTFAmmoPack") == 0 || strstr(networkName, "item_healthkit_") || strstr(networkName, "item_ammopack_")) ||
            (strcmp(networkName, "CBaseAnimating") == 0 && (H::Entities->IsHealthPack(pBase) || H::Entities->IsAmmoPack(pBase))))
        {
            if (strstr(networkName, "debris") && !strstr(networkName, "item_")) continue;
            if (!CFG::ESP_Pickups) continue;

            isPickup = true;
            bool isAmmo = (strcmp(networkName, "CTFAmmoPack") == 0 || strstr(networkName, "ammopack") || H::Entities->IsAmmoPack(pBase));
            name = isAmmo ? "Ammo" : "Medkit";
            clr = CFG::ESP_PickupsColor;
            drawESP = CFG::ESP_Pickups;
        }
        else if (strcmp(networkName, "CCaptureFlag") == 0 || strcmp(networkName, "CTFItemTeamFlag") == 0 ||
            strcmp(networkName, "CItemTeamFlag") == 0 || strcmp(networkName, "item_teamflag") == 0 ||
            strcmp(networkName, "CTeamControlPoint") == 0 || strcmp(networkName, "CFuncTrackTrain") == 0)
        {
            if (CFG::ESP_Team && isTeammate) continue;
            isFlag = true;

            if (strstr(networkName, "flag") || strstr(networkName, "Flag"))
                name = "Intel";
            else if (strcmp(networkName, "CTeamControlPoint") == 0)
                name = "Control Point";
            else
                name = "Payload Cart";

            drawESP = CFG::ESP_CaptureFlag;
            clr = CFG::ESP_BoxCaptureColor;
        }
        else continue;

        Vec3 origin = pBase->GetAbsOrigin();
        float dist = (pLocal->GetShootPos() - origin).Length() / 39.37f;

        Vec3 mins = pBase->m_vecMins();
        Vec3 maxs = pBase->m_vecMaxs();

        if (!IsFiniteVec(mins) || !IsFiniteVec(maxs) || !IsFiniteVec(origin))
            continue;

        if (mins.Length() < 0.1f || maxs.Length() < 0.1f) {
            if (auto pAnim = pBase->As<C_BaseAnimating>()) {
                auto pModel = pAnim->GetModel();
                if (pModel) {
                    auto pStudio = I::ModelInfoClient->GetStudiomodel(pModel);
                    if (pStudio) {
                        mins = pStudio->hull_min;
                        maxs = pStudio->hull_max;
                    }
                }
            }

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

        int sw, sh;
        I::EngineClient->GetScreenSize(sw, sh);

        BBox2D bbox = H::Draw->ComputeBBox2D(points, 8);

        if (!bbox.valid) {
            if (CFG::ESP_Offscreen && isEnemy && isPlayer &&
                (CFG::ESP_Offscreen_MaxDist <= 0.0f || dist <= CFG::ESP_Offscreen_MaxDist)) {
                PlayerArrow(pPlayer, CFG::ESP_OffscreenColor);
            }
            continue;
        }

        if (!bbox.IsPartiallyOnScreen(sw, sh)) {
            if (CFG::ESP_Offscreen && isEnemy && isPlayer &&
                (CFG::ESP_Offscreen_MaxDist <= 0.0f || dist <= CFG::ESP_Offscreen_MaxDist)) {
                PlayerArrow(pPlayer, clr);
            }
            continue;
        }

        int boxLeft = static_cast<int>(bbox.left);
        int boxTop = static_cast<int>(bbox.top);
        int boxW = bbox.GetWidth();
        int boxH = bbox.GetHeight();

        Vec3 screenPts[8];
        bool has3DPoints = true;
        for (int k = 0; k < 8; ++k) {
            if (!H::Draw->W2S(points[k], screenPts[k])) {
                has3DPoints = false;
            }
        }

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

            float distScale = H::Draw->GetDistanceScale(dist, 500.0f);
            distScale = std::clamp(distScale, 0.7f, 1.0f);

            if (draw_box) {
                bool useAA = (dist < 1500.0f);

                Color_t boxColor = clr;
                if (isPickup) boxColor = CFG::ESP_PickupsBoxColor;
                if (isFlag) boxColor = CFG::ESP_BoxCaptureColor;
                if (isBuilding) boxColor = CFG::ESP_BuildColor;

                switch (CFG::ESP_BoxType) {
                case 0: DrawBox2D(boxLeft, boxTop, boxW, boxH, boxColor); break;
                case 1:
                    if (has3DPoints) {
                        DrawBox3D(screenPts, boxColor, useAA);
                    }
                    else {
                        DrawBox2D(boxLeft, boxTop, boxW, boxH, boxColor);
                    }
                    break;
                case 2: DrawBoxCorner(boxLeft, boxTop, boxW, boxH, boxColor); break;
                default: break;
                }
            }

            if (draw_name && !name.empty()) {
                const CFont& fontObj = H::Fonts->Get(EFonts::ESP);
                HFont font = fontObj.m_dwFont;

                Color_t nameClr = CFG::ESP_NameColor;
                if (isPickup) nameClr = CFG::ESP_PickupsNameColor;
                if (isFlag) nameClr = CFG::ESP_NameCaptureColor;
                if (isBuilding) nameClr = CFG::ESP_BuildColor;

                H::Draw->Text(bbox.GetCenterX(), boxTop - 15, font, nameClr, ALIGN_CENTER_H, name.c_str());
            }

            if (CFG::ESP_Health && max_health > 0 && (isPlayer || isBuilding)) {
                int animID = i * 10000 + 1;
                float animatedHealth = H::Draw->GetAnimatedValue(animID, static_cast<float>(health), 150.0f);

                int healthType = CFG::ESP_HealthType;
                int position = CFG::ESP_HealthBarPosition;
                bool hasBar = (healthType == 0 || healthType == 2);

                int barX, barY, barW, barH;
                bool isVertical = (position == 0 || position == 1);

                if (isVertical) {
                    barW = 4;
                    barH = boxH + 2;
                    barY = boxTop - 1;
                    barX = (position == 0) ? boxLeft - 6 : boxLeft + boxW + 2;
                }
                else {
                    barW = boxW + 2;
                    barH = 4;
                    barX = boxLeft - 1;
                    barY = (position == 2) ? boxTop - 6 : boxTop + boxH + 2;
                }

                if (hasBar) {
                    H::Draw->RectFilled(barX, barY, barW, barH, CFG::Color_HealthBarBG);

                    if (animatedHealth > 0) {
                        float ratio = animatedHealth / static_cast<float>(max_health);
                        ratio = std::min(1.0f, ratio);

                        int fillW, fillH, fillX, fillY;

                        if (isVertical) {
                            fillW = 2;
                            fillH = static_cast<int>(std::round((barH - 2) * ratio));
                            fillX = barX + 1;
                            fillY = barY + (barH - 1) - fillH;
                        }
                        else {
                            fillW = static_cast<int>(std::round((barW - 2) * ratio));
                            fillH = 2;
                            fillX = barX + 1;
                            fillY = barY + 1;
                        }

                        Color_t healthColor = (health > max_health) ? CFG::Color_Overheal : GetHealthBarColor(health, max_health);

                        if ((isVertical && fillH > 0) || (!isVertical && fillW > 0)) {
                            H::Draw->RectFilled(fillX, fillY, fillW, fillH, healthColor);
                        }
                    }
                }

                if (healthType == 1 || healthType == 2) {
                    const CFont& fontObj = H::Fonts->Get(EFonts::ESP);
                    HFont font = fontObj.m_dwFont;
                    std::string healthStr = std::to_string(health);

                    int textW, textH;
                    H::Draw->GetTextSize(font, healthStr.c_str(), textW, textH);

                    int textX, textY;

                    switch (position) {
                    case 0:
                        textX = (hasBar ? barX : boxLeft) - textW - 2;
                        textY = boxTop;
                        break;
                    case 1:
                        textX = (hasBar ? barX + barW : boxLeft + boxW) + 2;
                        textY = boxTop;
                        break;
                    case 2:
                        textX = bbox.GetCenterX() - (textW / 2);
                        textY = (hasBar ? barY : boxTop) - textH - 2;
                        break;
                    case 3:
                        textX = bbox.GetCenterX() - (textW / 2);
                        textY = (hasBar ? barY + barH : boxTop + boxH) + 2;
                        break;
                    default:
                        textX = boxLeft - textW - 2;
                        textY = boxTop;
                        break;
                    }

                    H::Draw->Text(textX, textY, font, CFG::Color_HealthText, ALIGN_DEFAULT, healthStr.c_str());
                }
            }

            if (isPlayer && CFG::ESP_Skeleton) {
                Color_t skeletonColor = CFG::Color_Skeleton;

                DrawSkeleton(pPlayer, skeletonColor);

                if (CFG::ESP_Skeleton_OnShot) {
                    auto itShot = m_mBonesOnShot.find(pPlayer);
                    if (itShot != m_mBonesOnShot.end()) {
                        float storedTime = itShot->second.second;
                        if (I::GlobalVars->realtime - storedTime <= 1.5f) {
                            const BoneMatrixes_t& snapBM = itShot->second.first;
                            
                            auto pModel = pPlayer->GetModel();
                            if (pModel) {
                                auto pStudio = I::ModelInfoClient->GetStudiomodel(pModel);
                                if (pStudio) {
                                    for (int i = 0; i < pStudio->numbones; i++) {
                                        mstudiobone_t* pBone = pStudio->pBone(i);
                                        if (!pBone || pBone->parent < 0)
                                            continue;

                                        const char* boneName = pBone->pszName();
                                        if (!boneName)
                                            continue;

                                        if (strstr(boneName, "finger") || strstr(boneName, "thumb") ||
                                            strstr(boneName, "middle") || strstr(boneName, "ring") ||
                                            strstr(boneName, "pinky") || strstr(boneName, "foot") ||
                                            strstr(boneName, "toe") || strstr(boneName, "hat") ||
                                            strstr(boneName, "cap") || strstr(boneName, "attach") ||
                                            strstr(boneName, "weapon") || strstr(boneName, "prop"))
                                            continue;

                                        int parentBone = pBone->parent;

                                        Vec3 vBone = GetBonePosition(pPlayer, i, &snapBM);
                                        Vec3 vParent = GetBonePosition(pPlayer, parentBone, &snapBM);

                                        if (!IsFiniteVec(vBone) || !IsFiniteVec(vParent))
                                            continue;

                                        if (vBone.Length() < 0.1f || vParent.Length() < 0.1f)
                                            continue;

                                        float dist = (vBone - vParent).Length();
                                        if (dist < 1.0f || dist > 300.0f)
                                            continue;

                                        Vec3 vScreen1, vScreen2;
                                        if (!H::Draw->W2S(vBone, vScreen1) || !H::Draw->W2S(vParent, vScreen2))
                                            continue;

                                        if (!IsFiniteVec(vScreen1) || !IsFiniteVec(vScreen2))
                                            continue;

                                        DrawOutlinedLine(vScreen1, vScreen2, CFG::Color_Skeleton_OnShot, CFG::Color_ESP_Outline);
                                    }
                                }
                            }
                        }
                        else {
                            m_mBonesOnShot.erase(itShot);
                            m_mBoneHitboxesOnShot.erase(pPlayer);
                        }
                    }
                }

                if (CFG::ESP_Skeleton_OnHit) {
                    auto itHit = m_mBonesOnHit.find(pPlayer);
                    if (itHit != m_mBonesOnHit.end()) {
                        float storedTime = itHit->second.second;
                        if (I::GlobalVars->realtime - storedTime <= 1.5f) {
                            const BoneMatrixes_t& snapBM = itHit->second.first;
                            
                            auto pModel = pPlayer->GetModel();
                            if (pModel) {
                                auto pStudio = I::ModelInfoClient->GetStudiomodel(pModel);
                                if (pStudio) {
                                    for (int i = 0; i < pStudio->numbones; i++) {
                                        mstudiobone_t* pBone = pStudio->pBone(i);
                                        if (!pBone || pBone->parent < 0)
                                            continue;

                                        const char* boneName = pBone->pszName();
                                        if (!boneName)
                                            continue;

                                        if (strstr(boneName, "finger") || strstr(boneName, "thumb") ||
                                            strstr(boneName, "middle") || strstr(boneName, "ring") ||
                                            strstr(boneName, "pinky") || strstr(boneName, "foot") ||
                                            strstr(boneName, "toe") || strstr(boneName, "hat") ||
                                            strstr(boneName, "cap") || strstr(boneName, "attach") ||
                                            strstr(boneName, "weapon") || strstr(boneName, "prop"))
                                            continue;

                                        int parentBone = pBone->parent;

                                        Vec3 vBone = GetBonePosition(pPlayer, i, &snapBM);
                                        Vec3 vParent = GetBonePosition(pPlayer, parentBone, &snapBM);

                                        if (!IsFiniteVec(vBone) || !IsFiniteVec(vParent))
                                            continue;

                                        if (vBone.Length() < 0.1f || vParent.Length() < 0.1f)
                                            continue;

                                        float dist = (vBone - vParent).Length();
                                        if (dist < 1.0f || dist > 300.0f)
                                            continue;

                                        Vec3 vScreen1, vScreen2;
                                        if (!H::Draw->W2S(vBone, vScreen1) || !H::Draw->W2S(vParent, vScreen2))
                                            continue;

                                        if (!IsFiniteVec(vScreen1) || !IsFiniteVec(vScreen2))
                                            continue;

                                        DrawOutlinedLine(vScreen1, vScreen2, CFG::Color_Skeleton_OnHit, CFG::Color_ESP_Outline);
                                    }
                                }
                            }

                            auto itHitPos = m_mHitPosOnHit.find(pPlayer);
                            if (itHitPos != m_mHitPosOnHit.end()) {
                                Vec3 hitPos = itHitPos->second.first;
                                Vec3 scr;
                                if (H::Draw->W2S(hitPos, scr)) {
                                    H::Draw->CircleFilled(static_cast<int>(scr.x), static_cast<int>(scr.y), 4, 16, CFG::Color_Skeleton_OnHit);
                                }
                            }
                        }
                        else {
                            m_mBonesOnHit.erase(itHit);
                            m_mBoneHitboxesOnHit.erase(pPlayer);
                            m_mHitPosOnHit.erase(pPlayer);
                        }
                    }
                }
            }

            if (isPlayer && CFG::ESP_Skeleton_Bounds) {
                DrawBounds(pPlayer, CFG::Color_Skeleton);
            }

            if (isPlayer && CFG::ESP_Skeleton_AimPoints) {
                auto it = m_mAimbotAimPoints.find(pPlayer);
                if (it != m_mAimbotAimPoints.end()) {
                    float stored = it->second.second;
                    if (I::GlobalVars->realtime - stored <= 1.0f) {
                        Vec3 aimPos = it->second.first;
                        Vec3 vScreen;
                        if (H::Draw->W2S(aimPos, vScreen)) {
                            H::Draw->CircleFilled(static_cast<int>(vScreen.x), static_cast<int>(vScreen.y), 4, 16, Color_t(255, 0, 0, 255));
                            H::Draw->Circle(static_cast<int>(vScreen.x), static_cast<int>(vScreen.y), 4, 16, Color_t(0, 0, 0, 255));
                            continue;
                        }
                    }
                    else {
                        m_mAimbotAimPoints.erase(it);
                    }
                }

                if (m_mBoneHitboxes.find(pPlayer) == m_mBoneHitboxes.end())
                    return;

                const auto& hitboxPoints = m_mBoneHitboxes[pPlayer];

                for (size_t i = 0; i < hitboxPoints.size(); i += 9)
                {
                    if (i >= hitboxPoints.size())
                        break;

                    const Vec3& centerPoint = hitboxPoints[i];

                    Vec3 vScreen;
                    if (!H::Draw->W2S(centerPoint, vScreen))
                        continue;

                    H::Draw->CircleFilled(static_cast<int>(vScreen.x), static_cast<int>(vScreen.y),
                        3, 12, Color_t(255, 0, 0, 255));
                    H::Draw->Circle(static_cast<int>(vScreen.x), static_cast<int>(vScreen.y),
                        3, 12, Color_t(0, 0, 0, 255));
                }
            }
        }

        if (isPlayer && drawESP) {
            int playerClass = pPlayer->m_iClass();
            const CFont& fontObj = H::Fonts->Get(EFonts::ESP);
            HFont font = fontObj.m_dwFont;

            int rightTextX = boxLeft + boxW + 5;
            int rightTextY = boxTop;
            int fontHeight = H::Fonts->GetFontHeight(EFonts::ESP);

            if (CFG::ESP_Conds) {
                std::vector<std::string> conds;
                if (InCond(pPlayer, 4)) conds.push_back("Cloaked");
                if (InCond(pPlayer, 3)) conds.push_back("Disguised");
                if (InCond(pPlayer, 5)) conds.push_back("Uber");
                if (InCond(pPlayer, 22)) conds.push_back("Burning");
                if (InCond(pPlayer, 7)) conds.push_back("Taunt");

                for (const auto& c : conds) {
                    H::Draw->Text(rightTextX, rightTextY, font, CFG::Color_CondsText, ALIGN_DEFAULT, c.c_str());
                    rightTextY += fontHeight + 1;
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
                        rightTextY += fontHeight + 1;
                    }
                }
                if (CFG::ESP_Debuffs) {
                    for (const auto& d : debuffs) {
                        H::Draw->Text(rightTextX, rightTextY, font, Color_t(255, 0, 0, 255), ALIGN_DEFAULT, d.c_str());
                        rightTextY += fontHeight + 1;
                    }
                }
            }

            if (CFG::ESP_DistanceEnemy && isEnemy) {
                std::string distStr = std::to_string(static_cast<int>(dist)) + " m";
                if (CFG::ESP_DistancePosition == 0) {
                    H::Draw->Text(rightTextX, rightTextY, font, CFG::ESP_DistanceColor, ALIGN_DEFAULT, distStr.c_str());
                    rightTextY += fontHeight + 1;
                }
                else {
                    H::Draw->Text(bbox.GetCenterX(), boxTop + boxH + 5, font, CFG::ESP_DistanceColor, ALIGN_CENTER_H, distStr.c_str());
                }
            }

            if (CFG::ESP_Ping) {
                static int pingOffset = NetVars::GetNetVar("CPlayerResource", "m_iPing");
                int ping = pResource ? *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pResource) + pingOffset + i * sizeof(int)) : 0;
                std::string pingStr = std::to_string(ping) + " ms";
                H::Draw->Text(rightTextX, rightTextY, font, CFG::ESP_PingColor, ALIGN_DEFAULT, pingStr.c_str());
                rightTextY += fontHeight + 1;
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
                    DrawSmoothBoneLine(scrStart, scrEnd, CFG::ESP_SniperLinesColor, false);
                }
            }

            if (CFG::ESP_Tracer && isEnemy) {
                Vec3 head = GetHitboxPosition(pPlayer, 0);
                Vec3 scrHead;
                if (H::Draw->W2S(head, scrHead)) {
                    Vec3 center(static_cast<float>(sw) / 2.0f, static_cast<float>(sh) / 2.0f, 0.0f);
                    DrawSmoothBoneLine(center, scrHead, CFG::ESP_TracerColor, true);
                }
            }

            if (CFG::ESP_Uber && pPlayer->m_iClass() == TF_CLASS_MEDIC) {
                if (auto pWeapon = pPlayer->GetWeaponFromSlot(1)) {
                    const CFont& smallFontObj = H::Fonts->Get(EFonts::ESP_SMALL);
                    HFont smallFont = smallFontObj.m_dwFont;
                    H::Draw->TextF(rightTextX, rightTextY, smallFont, CFG::ESP_UberStatusColor, ALIGN_DEFAULT,
                        "%d%%", static_cast<int>(pWeapon->As<C_WeaponMedigun>()->m_flChargeLevel() * 100.0f));
                    rightTextY += H::Fonts->GetFontHeight(EFonts::ESP_SMALL) + 1;
                }
            }

            if (CFG::ESP_UberBar && pPlayer->m_iClass() == TF_CLASS_MEDIC) {
                if (auto pWeapon = pPlayer->GetWeaponFromSlot(1)) {
                    auto pMedigun = pWeapon->As<C_WeaponMedigun>();
                    if (auto flCharge = pMedigun->m_flChargeLevel()) {
                        int nBarH = 2;
                        int nDrawY = boxTop + boxH + nBarH + 1;
                        float flFillW = Math::RemapValClamped(flCharge, 0.0f, 1.0f, 0.0f, static_cast<float>(boxW));

                        H::Draw->Rect(boxLeft - 1, nDrawY - 1, static_cast<int>(flFillW) + 2, nBarH + 2, CFG::Color_ESP_Outline);
                        H::Draw->RectFilled(boxLeft, nDrawY, static_cast<int>(flFillW), nBarH, CFG::ESP_UberBarColor);

                        if (pMedigun->m_iItemDefinitionIndex() == Medic_s_TheVaccinator) {
                            if (flCharge >= 0.25f)
                                H::Draw->RectFilled(boxLeft + static_cast<int>(static_cast<float>(boxW) * 0.25f) - 1, nDrawY, 2, nBarH, CFG::Color_ESP_Outline);
                            if (flCharge >= 0.5f)
                                H::Draw->RectFilled(boxLeft + static_cast<int>(static_cast<float>(boxW) * 0.5f) - 1, nDrawY, 2, nBarH, CFG::Color_ESP_Outline);
                            if (flCharge >= 0.75f)
                                H::Draw->RectFilled(boxLeft + static_cast<int>(static_cast<float>(boxW) * 0.75f) - 1, nDrawY, 2, nBarH, CFG::Color_ESP_Outline);
                        }
                    }
                }
            }
        }
    }

    if (CFG::Aimbot_DrawFOV) {
        DrawFOVCircle(CFG::Aimbot_FOV, CFG::Aimbot_FOVColor);
    }
}

void CESP::Init()
{
    if (m_bInitialized) return;
    m_bInitialized = true;
}

void CESP::DrawBounds(C_TFPlayer* pPlayer, const Color_t& clr)
{
    if (!pPlayer || pPlayer->IsDormant())
        return;

    auto pModel = pPlayer->GetModel();
    if (!pModel) return;

    auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR) return;

    auto pSet = pHDR->pHitboxSet(pPlayer->m_nHitboxSet());
    if (!pSet) return;

    matrix3x4_t aBones[128];
    bool gotBones = pPlayer->SetupBones(aBones, 128, BONE_USED_BY_HITBOX, I::GlobalVars->curtime);
    if (!gotBones) {
        auto it = m_mBones.find(pPlayer);
        if (it != m_mBones.end()) {
            for (int b = 0; b < 128; ++b)
                for (int r = 0; r < 3; ++r)
                    for (int c = 0; c < 4; ++c)
                        aBones[b][r][c] = it->second.BoneMatrix[b][r][c];
            gotBones = true;
        }
    }

    if (!gotBones)
        return;

    const std::pair<int, int> edges[] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    for (int i = 0; i < pSet->numhitboxes; ++i)
    {
        auto pBox = pSet->pHitbox(i);
        if (!pBox) continue;

        Vec3 ext = pBox->bbmax - pBox->bbmin;
        if (ext.Length() < 0.001f) continue;
        if (pBox->bone < 0 || pBox->bone >= 128) continue;

        Vec3 localCorners[8] = {
            { pBox->bbmin.x, pBox->bbmin.y, pBox->bbmin.z },
            { pBox->bbmin.x, pBox->bbmax.y, pBox->bbmin.z },
            { pBox->bbmax.x, pBox->bbmax.y, pBox->bbmin.z },
            { pBox->bbmax.x, pBox->bbmin.y, pBox->bbmin.z },
            { pBox->bbmin.x, pBox->bbmin.y, pBox->bbmax.z },
            { pBox->bbmin.x, pBox->bbmax.y, pBox->bbmax.z },
            { pBox->bbmax.x, pBox->bbmax.y, pBox->bbmax.z },
            { pBox->bbmax.x, pBox->bbmin.y, pBox->bbmax.z }
        };

        Vec3 corners[8];
        for (int j = 0; j < 8; ++j) {
            Math::VectorTransform(localCorners[j], aBones[pBox->bone], corners[j]);
        }

        Vec3 scr[8];
        bool valid[8] = { false };
        int validCount = 0;
        for (int j = 0; j < 8; ++j) {
            Vec3 tmp;
            if (H::Draw->W2S(corners[j], tmp)) {
                if (std::isfinite(tmp.x) && std::isfinite(tmp.y)) {
                    scr[j] = tmp;
                    valid[j] = true;
                    ++validCount;
                }
            }
        }

        if (validCount < 2)
            continue;

        for (auto& e : edges) {
            int a = e.first;
            int b = e.second;
            if (a < 0 || a >= 8 || b < 0 || b >= 8) continue;
            if (valid[a] && valid[b]) {
                DrawThinLine(scr[a], scr[b], clr);
            }
        }
    }
}

void CESP::Shutdown()
{
    m_mBones.clear();
    m_mBoneHitboxes.clear();
    m_mBonesOnShot.clear();
    m_mBoneHitboxesOnShot.clear();
    m_mBonesOnHit.clear();
    m_mBoneHitboxesOnHit.clear();
    m_mHitPosOnHit.clear();
    m_mAimbotAimPoints.clear();
    m_AnimatedHealth.clear();
    m_InitialAppearTime.clear();
}

CESP gESP;