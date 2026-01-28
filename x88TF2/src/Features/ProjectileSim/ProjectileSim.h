#include "../src/SDK/SDK.h"  // Assume this includes necessary classes like C_TFCompoundBow, C_TFPipebombLauncher, etc.

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
    // Add more as needed (e.g., JAR, JAR_MILK from UC threads)
};

struct ProjectileInfo
{
    SimProjType m_type = SimProjType::NONE;
    Vec3 m_pos{};           // Spawn position
    Vec3 m_ang{};           // Launch angles
    float m_speed = 0.0f;   // Initial speed
    float m_gravity_mod = 1.0f;
    bool no_spin = false;
    float m_radius = 0.0f;  // Explosion radius (0 = no splash)
};

class CProjectileSim
{
public:
    bool GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out);
    bool GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out, int flags);

    bool Init(const ProjectileInfo& info, bool no_vec_up = false);
    void RunTick();
    Vec3 GetOrigin();
    Vec3 GetPosition() { return GetOrigin(); }  // Alias for old code
    Vec3 GetImpactOrigin() const;
    bool HasImpacted() const;
    float GetExplosionRadius() const;
    bool CanHitWithSplash(const Vec3& targetPos) const;

    // Legacy compatibility
    bool SimulateToTime(float flTime, Vec3& outPos, bool bTrace = true);
    bool TraceToTarget(const Vec3& targetPos, float& flTimeToImpact);

private:
    IPhysicsEnvironment* m_env = nullptr;
    IPhysicsObject* m_obj = nullptr;
    Vec3 m_impactPos{};
    bool m_hasImpacted = false;
    float m_radius = 0.0f;
    int m_tickCount = 0;
    static constexpr int MAX_TICKS = 500;
};

MAKE_SINGLETON_SCOPED(CProjectileSim, ProjectileSim, F);