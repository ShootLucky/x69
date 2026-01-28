#include "../SDK/SDK.h"
#include "../Features/Aimbot/AimbotHitscan/AimbotHitscan.h"

// Signature para fire_projectile
MAKE_SIGNATURE(FireProjectile, "client.dll", "48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B FA 48 89 9C 24", 0x0);

// Hook do fire_projectile - aqui roda hitscan aimbot
MAKE_HOOK(FireProjectile, Signatures::FireProjectile.Get(), C_BaseEntity*, __fastcall,
	void* rcx, C_TFPlayer* player)
{
	auto pLocal = H::Entities->GetLocal();

	if (!pLocal || !I::EngineClient->IsInGame())
	{
		return CALL_ORIGINAL(rcx, player);
	}

	// Se flag está setada, rodar hitscan
	if (G::bRunCmd && G::CurrentUserCmd)
	{
		auto pWeapon = H::Entities->GetWeapon();

		if (pWeapon)
		{
			// Verificar se é arma hitscan
			int weaponID = pWeapon->GetWeaponID();
			bool isHitscan = false;

			switch (weaponID)
			{
			case TF_WEAPON_MINIGUN:
			case TF_WEAPON_PISTOL:
			case TF_WEAPON_PISTOL_SCOUT:
			case TF_WEAPON_SCATTERGUN:
			case TF_WEAPON_SHOTGUN_PRIMARY:
			case TF_WEAPON_SHOTGUN_PYRO:
			case TF_WEAPON_SHOTGUN_SOLDIER:
			case TF_WEAPON_SHOTGUN_HWG:
			case TF_WEAPON_SNIPERRIFLE:
			case TF_WEAPON_SMG:
			case TF_WEAPON_REVOLVER:
				isHitscan = true;
				break;
			}

			// Se for hitscan, rodar aimbot
			if (isHitscan && F::AimbotHitscan)
			{
				F::AimbotHitscan->Run(G::CurrentUserCmd, pLocal, pWeapon);
			}
		}

		// Resetar flag
		G::bRunCmd = false;
	}

	return CALL_ORIGINAL(rcx, player);
}