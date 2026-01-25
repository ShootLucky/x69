#pragma once
#include "../src/SDK/SDK.h"
#include <vector>
#include <algorithm>

// Base target structure for all aimbot types
struct AimTarget_t
{
    C_BaseEntity* Entity = nullptr;
    Vec3 Position = {};
    Vec3 AngleTo = {};
    float FOVTo = 0.0f;
    float DistanceTo = 0.0f;
    int Priority = 0;
    float Score = FLT_MAX;
};

class CAimbotCommon
{
public:
    // Template sort function for any target type derived from AimTarget_t
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<AimTarget_t, T>>>
    void Sort(std::vector<T>& targets, int sortMode)
    {
        std::sort(targets.begin(), targets.end(), [&](const T& a, const T& b)
            {
                switch (sortMode)
                {
                case 0: // FOV
                    return a.FOVTo < b.FOVTo;
                case 1: // Distance
                    return a.DistanceTo < b.DistanceTo;
                case 2: // Custom Score
                    return a.Score < b.Score;
                default:
                    return a.FOVTo < b.FOVTo;
                }
            });
    }

    // Calculate FOV between two angles
    inline float CalculateFOV(const Vec3& vFrom, const Vec3& vTo)
    {
        Vec3 vDelta = vFrom - vTo;
        Math::ClampAngles(vDelta);
        return sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y);
    }

    // Calculate angle from point to point
    inline Vec3 CalcAngle(const Vec3& vFrom, const Vec3& vTo)
    {
        return Math::CalcAngle(vFrom, vTo);
    }

    // Check if entity is visible from position
    bool IsVisible(C_BaseEntity* pEntity, const Vec3& vFrom, const Vec3& vTo)
    {
        if (!pEntity)
            return false;

        trace_t trace;
        CTraceFilterWorldAndPropsOnly filter;

        Ray_t ray;
        ray.Init(vFrom, vTo);

        I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

        return (trace.m_pEnt == pEntity || trace.fraction >= 0.99f);
    }

    // Get velocity prediction offset
    Vec3 GetVelocityPrediction(C_BaseEntity* pEntity, float flTime)
    {
        if (!pEntity)
            return Vec3(0, 0, 0);

        auto pPlayer = pEntity->As<C_TFPlayer>();
        if (!pPlayer)
            return Vec3(0, 0, 0);

        Vec3 vVelocity = pPlayer->m_vecVelocity();

        // Apply gravity if in air
        if (!(pPlayer->m_fFlags() & FL_ONGROUND))
        {
            float flGravity = 800.0f; // TF2 gravity
            vVelocity.z -= flGravity * flTime * 0.5f;
        }

        return vVelocity * flTime;
    }

    // Get network latency
    float GetLatency()
    {
        INetChannelInfo* pNetChan = I::EngineClient->GetNetChannelInfo();
        if (!pNetChan)
            return 0.0f;

        return pNetChan->GetLatency(FLOW_OUTGOING) + pNetChan->GetLatency(FLOW_INCOMING);
    }

    // Get lerp time
    float GetLerpTime()
    {
        auto pLerpCvar = I::CVar->FindVar("cl_interp");
        return pLerpCvar ? pLerpCvar->GetFloat() : 0.0f;
    }

    // Get total prediction time
    float GetPredictionTime()
    {
        return GetLatency() + GetLerpTime();
    }

    // Smooth angle transition
    Vec3 SmoothAngles(const Vec3& vCurrent, const Vec3& vTarget, float fSmoothing)
    {
        if (fSmoothing <= 1.0f)
            return vTarget;

        Vec3 vDelta = vTarget - vCurrent;
        Math::ClampAngles(vDelta);

        return vCurrent + (vDelta / fSmoothing);
    }

    // Fix movement after angle change
    void FixMovement(CUserCmd* pCmd, const Vec3& vOldAngles)
    {
        Vec3 vMove(pCmd->forwardmove, pCmd->sidemove, pCmd->upmove);
        float fSpeed = sqrtf(vMove.x * vMove.x + vMove.y * vMove.y);

        if (fSpeed < 0.1f)
            return;

        Vec3 vMoveAngles;
        Math::VectorAngles(vMove, vMoveAngles);

        float fYawDelta = pCmd->viewangles.y - vOldAngles.y;
        vMoveAngles.y -= fYawDelta;

        Vec3 vNewMove;
        Math::AngleVectors(vMoveAngles, &vNewMove);
        vNewMove *= fSpeed;

        pCmd->forwardmove = vNewMove.x;
        pCmd->sidemove = vNewMove.y;
    }

    // Clamp angles
    inline void ClampAngles(Vec3& vAngles)
    {
        Math::ClampAngles(vAngles);
    }

    // Normalize angle
    inline float NormalizeAngle(float flAngle)
    {
        while (flAngle > 180.0f)
            flAngle -= 360.0f;
        while (flAngle < -180.0f)
            flAngle += 360.0f;
        return flAngle;
    }
};

MAKE_SINGLETON_SCOPED(CAimbotCommon, AimbotCommon, F);