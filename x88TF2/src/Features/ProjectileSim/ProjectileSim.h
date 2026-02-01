#pragma once
#include "../src/SDK/SDK.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum class SimProjType : uint8_t
{
    NONE = 0,
    ROCKET,
    PIPEBOMB,
    PIPEBOMB_REMOTE,
    PIPEBOMB_PRACTICE,
    CANNONBALL,
    FLARE,
    ARROW,
    SYRINGE,
    ENERGY_BALL,
};

struct ProjectileInfo
{
    SimProjType m_type = SimProjType::NONE;
    Vec3 m_pos{};
    Vec3 m_ang{};
    float m_speed = 0.0f;
    float m_gravity_mod = 1.0f;
    bool no_spin = false;
    float m_radius = 0.0f;
};

struct TrajectoryPoint
{
    Vec3 position;
    Vec3 velocity;
    float time;
    int tickNum;
};

struct ArcSolution
{
    Vec3 angles;
    float timeToTarget;
    float apexHeight;
    bool isValid;
    std::vector<TrajectoryPoint> trajectory;
    Vec3 impactPoint;
    float splashDamagePercent;
};

class CProjectileSim
{
public:
    bool GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out);
    bool GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out, int flags);

    bool Init(const ProjectileInfo& info, bool no_vec_up = false);
    void RunTick();
    Vec3 GetOrigin();
    Vec3 GetPosition() { return GetOrigin(); }
    Vec3 GetImpactOrigin() const;
    bool HasImpacted() const;
    float GetExplosionRadius() const;

    bool CanHitWithSplash(const Vec3& targetPos) const;
    float CalculateSplashDamage(const Vec3& targetPos, float maxDamage = 112.0f) const;
    Vec3 FindOptimalSplashPoint(const Vec3& targetPos, float searchRadius = 150.0f) const;
    bool GetSplashCoverage(const Vec3& impactPoint, std::vector<Vec3>& outCoveredPoints, int resolution = 8) const;

    bool CalculateArcShot(
        const Vec3& startPos,
        const Vec3& targetPos,
        float initialSpeed,
        float gravityMod,
        ArcSolution& outSolution,
        bool preferHighArc = false
    );

    bool FindArcSolutions(
        const Vec3& startPos,
        const Vec3& targetPos,
        float initialSpeed,
        float gravityMod,
        std::vector<ArcSolution>& outSolutions
    );

    bool ArcCanClearObstacles(
        const ArcSolution& arcSolution,
        const Vec3& targetPos,
        float clearanceHeight = 50.0f
    );

    bool FindBestArcSplashShot(
        const Vec3& startPos,
        const Vec3& targetPos,
        float initialSpeed,
        float gravityMod,
        ArcSolution& outBestSolution
    );

    bool SimulateFullTrajectory(
        const ProjectileInfo& info,
        std::vector<TrajectoryPoint>& outTrajectory,
        float maxTime = 5.0f
    );

    bool PredictImpactWithLatency(
        const Vec3& targetPos,
        const Vec3& targetVel,
        float latency,
        Vec3& outPredictedPos,
        float& outTimeToImpact
    );

    Vec3 InterpolateTrajectory(float time) const;

    bool TraceTrajectory(
        const std::vector<TrajectoryPoint>& trajectory,
        Vec3& outImpactPoint,
        float& outImpactTime,
        bool stopAtPlayers = false
    );

    struct SplashSurface
    {
        Vec3 position;
        Vec3 normal;
        float distance;
        float damageMultiplier;
    };

    bool FindNearbyWalls(
        const Vec3& targetPos,
        float searchRadius,
        std::vector<SplashSurface>& outSurfaces
    ) const;

    bool CalculateWallSplashShot(
        const Vec3& startPos,
        const Vec3& targetPos,
        const SplashSurface& wall,
        ArcSolution& outSolution
    );

    bool SimulateToTime(float flTime, Vec3& outPos, bool bTrace = true);
    bool TraceToTarget(const Vec3& targetPos, float& flTimeToImpact);

    const std::vector<TrajectoryPoint>& GetStoredTrajectory() const { return m_storedTrajectory; }
    float GetCurrentSimulationTime() const { return TICK_INTERVAL * m_tickCount; }
    int GetTickCount() const { return m_tickCount; }

private:
    IPhysicsEnvironment* m_env = nullptr;
    IPhysicsObject* m_obj = nullptr;
    Vec3 m_impactPos{};
    bool m_hasImpacted = false;
    float m_radius = 0.0f;
    int m_tickCount = 0;
    static constexpr int MAX_TICKS = 500;

    std::vector<TrajectoryPoint> m_storedTrajectory;

    float CalculateSplashFalloff(float distance, float radius) const;
    bool CheckLineOfSight(const Vec3& from, const Vec3& to, trace_t* outTrace = nullptr) const;
    Vec3 CalculateImpactNormal(const Vec3& impactPoint) const;

    bool SolveBallisticArc(
        const Vec3& origin,
        const Vec3& target,
        float speed,
        float gravity,
        float& outAngleLow,
        float& outAngleHigh
    ) const;

    bool TrajectoryIntersectsWorld(
        const std::vector<TrajectoryPoint>& trajectory,
        int startIdx,
        int endIdx,
        Vec3& outIntersection
    ) const;
};

MAKE_SINGLETON_SCOPED(CProjectileSim, ProjectileSim, F);