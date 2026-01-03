#pragma once

#include "../src/SDK/SDK.h"

struct ProjectileInfo
{
	ProjectileType_t m_type{};

	Vec3 m_pos{};
	Vec3 m_ang{};

	float m_speed{};
	float m_gravity_mod{};

	bool no_spin{};

	float m_radius{}; // New: Explosion radius
};

class CProjectileSim
{
public:
	bool GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out);
	bool Init(const ProjectileInfo& info, bool no_vec_up = false);
	void RunTick();
	Vec3 GetOrigin();
	Vec3 GetImpactOrigin() const; // Returns the position of first impact (if any)
	bool HasImpacted() const;     // Checks if an impact has occurred
	float GetExplosionRadius() const; // New: Returns the explosion radius for splash damage
	bool CanHitWithSplash(const Vec3& targetPos) const; // New: Checks if the target position can be hit by splash (distance <= radius and clear LOS)

private:
	IPhysicsEnvironment* m_env = nullptr; // Changed: Instance member
	IPhysicsObject* m_obj = nullptr;      // Changed: Instance member
	Vec3 m_impactPos{};                   // Stores the impact position
	bool m_hasImpacted = false;           // Flag for impact detection
	float m_radius = 0.0f;                // New: Explosion radius based on projectile type/attributes
	int m_tickCount = 0;                  // New: Tick counter for limiting simulation
	static constexpr int MAX_TICKS = 500; // New: Max ticks to prevent infinite simulation (adjust as needed)
};

MAKE_SINGLETON_SCOPED(CProjectileSim, ProjectileSim, F);