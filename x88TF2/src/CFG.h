// Fixed cfg.h - Atualizado com seed prediction para hitscan
#pragma once
#include "Utils/Config/Config.h"
namespace CFG
{
#pragma region Aimbot
	CFGVAR(Aimbot_Enable, false);
	CFGVAR(Aimbot_Key, -1);
	CFGVAR(Aimbot_KeyMode, 1);
	CFGVAR(Aimbot_Melee_KeyMode, 0);
	CFGVAR(Aimbot_Projectile_KeyMode, 0);
	CFGVAR(Aimbot_FOV, 45.f);
	CFGVAR(Aimbot_VisibleCheck, false);
	CFGVAR(Aimbot_TeamCheck, false);
	CFGVAR(Aimbot_Hitscan_Smoothing, 5.f);
	CFGVAR(Aimbot_Hitbox_Head, false);
	CFGVAR(Aimbot_Hitbox_Legs, false);
	CFGVAR(Aimbot_Hitbox_Arms, false);
	CFGVAR(Aimbot_Hitbox_Pelvis, false);
	CFGVAR(Aimbot_Hitbox_Body, false);
	CFGVAR(Aimbot_Hitbox_Buildings, false);
	CFGVAR(Aimbot_Projectile_Enable, false);
	CFGVAR(Aimbot_Hitscan_Multipoint_Scale, 0.f);
	CFGVAR(Aimbot_Projectile_FOV, 45.f);
	CFGVAR(Aimbot_Projectile_Smoothing, 5.f);
	CFGVAR(Aimbot_Projectile_TeamCheck, false);
	CFGVAR(Debug_SplashPoints, false);
	CFGVAR(Aimbot_Projectile_Mode, 0);
	CFGVAR(Aimbot_Projectile_AutoRelease, 0);            // 0-100% da carga
	CFGVAR(Aimbot_Projectile_DragCompensation, true);   // Compensar arrasto
	CFGVAR(Aimbot_Projectile_PredictTicks, 14);         // Ticks de predição
	CFGVAR(Aimbot_Projectile_Splash_Enable, false);
	CFGVAR(Aimbot_Projectile_Splash_Prefer, false);     // Preferir splash sobre direto
	CFGVAR(Aimbot_Projectile_Splash_Points, 32);
	CFGVAR(Aimbot_Projectile_Splash_Radius, 100);       // Porcentagem
	CFGVAR(Aimbot_Projectile_Huntsman_Headshot, true);
	CFGVAR(Aimbot_Projectile_Huntsman_Bodyaim, 50);     // Se HP < X%, mirar corpo
	CFGVAR(Aimbot_ActiveShoot, false);
	CFGVAR(Aimbot_ActiveLagRecords, false);
	CFGVAR(Aimbot_AutoShoot, false);
	CFGVAR(Aimbot_TargetStickies, false);
	CFGVAR(Aimbot_WaitForHeadshot, false);
	CFGVAR(Aimbot_AutoScope, false);
	CFGVAR(Aimbot_MinigunTapfire, false);
	CFGVAR(Aimbot_WaitForCharge, false);
	CFGVAR(Aimbot_Target_Players, false);
	CFGVAR(Aimbot_Target_Buildings, false);
	CFGVAR(Aimbot_Ignore_Friends, false);
	CFGVAR(Aimbot_Ignore_Invulnerable, false);
	CFGVAR(Aimbot_ActiveMelee, false);
	CFGVAR(Aimbot_Hitscan_Mode, 0);
	CFGVAR(Aimbot_Hitscan_Sort, 0);
	CFGVAR(Aimbot_TargetLagRecords, false);
	CFGVAR(Aimbot_WalkToTarget, false);
	CFGVAR(Aimbot_WhitelistTeammates, false);
	CFGVAR(Aimbot_BaimAfterShots, 0);
	CFGVAR(Aimbot_BaimAfterHealth, 0.18f);
	CFGVAR(Aimbot_Hitbox_Sort, 0);
	CFGVAR(Aimbot_Projectile_NoSpread, false);
	CFGVAR(Aimbot_Projectile_AutoDoubleDonk, false);
	CFGVAR(Aimbot_Projectile_AimPosition, 0);
	CFGVAR(Aimbot_Projectile_GroundStrafePrediction, false);
	CFGVAR(Aimbot_Projectile_AdvancedAirStrafe, false);
	CFGVAR(Aimbot_Projectile_RocketSplash, false);
	CFGVAR(Aimbot_Projectile_Sort, 0);
	CFGVAR(Aimbot_Projectile_PredictionMethod, 0);
	CFGVAR(Aimbot_Projectile_MaxSimulationTime, 1.5f);
	CFGVAR(Aimbot_Projectile_MaxTargets, 1);
	CFGVAR(Aimbot_Melee_Active, false);
	CFGVAR(Aimbot_Melee_AlwaysActive, false);
	CFGVAR(Aimbot_Melee_TargetLagRecords, false);
	CFGVAR(Aimbot_Melee_PredictSwing, false);
	CFGVAR(Aimbot_Melee_WalkToTarget, false);
	CFGVAR(Aimbot_Melee_WhipTeammates, false);
	CFGVAR(Aimbot_Melee_Key, 0);
	CFGVAR(Aimbot_Melee_Mode, 0);
	CFGVAR(Aimbot_Melee_Sort, 0);
	CFGVAR(Aimbot_Melee_FOV, 45.f);
	CFGVAR(Aimbot_Melee_Smoothing, 5.f);
	CFGVAR(Aimbot_Melee_PredictSwingTime, 0.18f);
	CFGVAR(Aimbot_Ignore_Invisible, false);
	CFGVAR(Aimbot_Ignore_Taunting, false);
	CFGVAR(Aimbot_Projectile_Active, false);
	CFGVAR(Aimbot_Projectile_Advanced_Head_Aim, false);
	CFGVAR(Aimbot_Projectile_Auto_Double_Donk, false);
	CFGVAR(Aimbot_Projectile_BBox_Multipoint, false);
	CFGVAR(Aimbot_Projectile_Max_Processing_Targets, 5);
	CFGVAR(Aimbot_Projectile_Max_Simulation_Time, 2.0f);
	CFGVAR(Aimbot_Projectile_Rocket_Splash, false);
	CFGVAR(Aimbot_Hitscan_Hitbox, 0);
	CFGVAR(Aimbot_Hitscan_Scan_Head, false);
	CFGVAR(Aimbot_Hitscan_Scan_Body, false);
	CFGVAR(Aimbot_Hitscan_Scan_Arms, false);
	CFGVAR(Aimbot_Hitscan_Scan_Legs, false);
	CFGVAR(Aimbot_Hitscan_Scan_Buildings, false);
	CFGVAR(Aimbot_Projectile_SplashBot, false);
	CFGVAR(Aimbot_Projectile_SplashPoints, 80.0f);
	CFGVAR(Aimbot_Projectile_SplashRadius, 1.0f);
	CFGVAR(Aimbot_Projectile_SplashTestPoints, 8);
	CFGVAR(Aimbot_Projectile_SplashMaxDist, 64.0f);
	CFGVAR(Aimbot_Projectile_SplashPrioritizeGround, true);
	CFGVAR(Aimbot_Projectile_SplashUseNN, false);
	CFGVAR(Aimbot_Active, false);
	CFGVAR(Aimbot_Hitscan_Active, false);
	CFGVAR(Aimbot_Hitscan_Target_LagRecords, false);
	CFGVAR(Aimbot_Melee_Target_LagRecords, false);
#pragma endregion

#pragma region ESP
	CFGVAR(ESP_Enable, false);
	CFGVAR(ESP_Box, false);
	CFGVAR(ESP_BoxType, 0);
	CFGVAR(ESP_Name, false);
	CFGVAR(ESP_Health, false);
	CFGVAR(ESP_HealthType, 0);
	CFGVAR(ESP_HealthBarPosition, 0);
	CFGVAR(ESP_HealthBarGradient, true);
	CFGVAR(ESP_HealthBarColor, Color_t(0, 255, 0, 255));
	CFGVAR(ESP_HealthBarGradientLow, Color_t(255, 0, 0, 255));
	CFGVAR(ESP_HealthBarGradientMid, Color_t(255, 255, 0, 255));
	CFGVAR(ESP_HealthBarGradientHigh, Color_t(0, 255, 0, 255));
	CFGVAR(ESP_Team, false);
	CFGVAR(ESP_CaptureFlag, false);
	CFGVAR(ESP_Build, false);
	CFGVAR(ESP_BuildOnlyEnemy, false);
	CFGVAR(ESP_HideCloaked, false);
	CFGVAR(ESP_LocalPlayer, false);
	CFGVAR(ESP_Offscreen, false);
	CFGVAR(ESP_Offscreen_Radius, 80.0f);
	CFGVAR(ESP_Offscreen_MaxDist, 0.0f);
	CFGVAR(ESP_Offscreen_Style, 0);
	CFGVAR(ESP_Offscreen_Filled, false);
	CFGVAR(ESP_Pickups, false);
	CFGVAR(ESP_PickupsBox, false);
	CFGVAR(ESP_PickupsName, false);
	CFGVAR(Aimbot_DrawFOV, false);
	CFGVAR(BulletTracer, false);
	CFGVAR(BulletTracer_Type, 0);
	CFGVAR(BulletTracer_Width, 1.0f);
	CFGVAR(BulletTracer_Speed, 0.0f);
	CFGVAR(BulletTracer_Length, 3.0f);
	CFGVAR(ESP_BoxCapture, false);
	CFGVAR(ESP_NameCapture, false);
	CFGVAR(Visuals_Draw_Movement_Path_Style, 0);
	CFGVAR(Logs_Enable, false);
	CFGVAR(Logs_Type, 0);
	CFGVAR(PlayersLogs_Type, 0);
	CFGVAR(Visuals_ThirdPerson_ScopedFov, 0.f);
	CFGVAR(Visuals_CustomFov_Amount, 0.f);
	CFGVAR(Visuals_CustomFov_Enable, false);
	CFGVAR(Visuals_RemoveFire, false);
	CFGVAR(Visuals_RemovePunch, false);
	CFGVAR(Visuals_RemoveScoped, false);
	CFGVAR(Visuals_RemoveScopedZoom, false);
	CFGVAR(Visuals_ViewModel_Enable, false);
	CFGVAR(Visuals_ViewModel_Forward, 0.f);
	CFGVAR(Visuals_ViewModel_Right, 0.f);
	CFGVAR(Visuals_ViewModel_Up, 0.f);
	CFGVAR(Visuals_Removals_Mode, 0);
	CFGVAR(Visuals_Rain, false);
	CFGVAR(Visual_Spectatorlist, false);
	CFGVAR(ESP_Arrows, false);
	CFGVAR(ESP_Conds, false);
	CFGVAR(ESP_SniperLines, false);
	CFGVAR(ESP_Tracer, false);
	CFGVAR(ESP_Uber, false);
	CFGVAR(ESP_UberBar, false);
	CFGVAR(Visuals_Disable_Dropped_Weapons, false);
	CFGVAR(Visuals_Disable_Wearables, false);
	CFGVAR(Visuals_Night_Mode, 0.f);
	CFGVAR(Visuals_Simple_Models, false);
	CFGVAR(Visuals_World_Modulation_Mode, 0);
	CFGVAR(Visuals_Chat_Player_List_Info, false);
	CFGVAR(Visuals_Flat_Textures, false);
	CFGVAR(ESP_DistanceEnemy, false);
	CFGVAR(ESP_Buffs, false);
	CFGVAR(ESP_Debuffs, false);
	CFGVAR(ESP_Ping, false);
	CFGVAR(ESP_KRDPlayer, false);
	CFGVAR(ESP_LagCompensation, false);
	CFGVAR(ESP_DistancePosition, 0);
	CFGVAR(ESP_Skeleton, false);
	CFGVAR(ESP_Skeleton_OnShot, false);
	CFGVAR(ESP_Skeleton_OnHit, false);
	CFGVAR(ESP_Skeleton_Bounds, false);
	CFGVAR(ESP_Skeleton_AimPoints, false);
	CFGVAR(ESP_Skeleton_Backtrack, false);
	CFGVAR(ESP_Skeleton_BacktrackType, 0);
#pragma endregion

#pragma region Materials
	CFGVAR(Materials_Active, false);

	// ================= PLAYERS =================
	CFGVAR(Materials_Players_Active, false);
	CFGVAR(Materials_Players_Material, 0);          // 0=None, 1=Flat, 2=Shaded, 3=Glossy, 4=Glow, 5=Plastic, 6=Fresnel
	CFGVAR(Materials_Players_Alpha, 1.0f);
	CFGVAR(Materials_Players_IgnoreDepth, false);   // Wallhack
	CFGVAR(Materials_Players_TwoModels, 0);         // 0=None, 1=Overlay, 2=Killstreak, 3=Exorcism, 4=FlatOverlay
	CFGVAR(Materials_Players_OverlayAlpha, 0.5f);   // Alpha do overlay

	// Overlay (usa materiais base em vez dos especiais)
	CFGVAR(Materials_Players_Overlay, false);      // NOVO: Ativa overlay
	CFGVAR(Materials_Players_OverlayMaterial, 1);  // NOVO: Material do overlay (0-6, mesma lista)

	// Filtros
	CFGVAR(Materials_Players_Ignore_Local, false);
	CFGVAR(Materials_Players_Ignore_Teammates, false);
	CFGVAR(Materials_Players_Ignore_Enemies, false);
	CFGVAR(Materials_Players_Ignore_Friends, false);
	CFGVAR(Materials_Players_Ignore_LagRecords, false);
	CFGVAR(Materials_Players_Show_Teammate_Medics, false);
	CFGVAR(Materials_Players_LagRecords_Style, 0); // 0=Flat, 1=Shaded

	// REMOVIDO: Materials_Players_HiddenMaterial
	// REMOVIDO: Materials_Players_TwoModels (substituído por Overlay)

	// ================= BUILDINGS =================
	CFGVAR(Materials_Buildings_Active, false);
	CFGVAR(Materials_Buildings_Material, 0);
	CFGVAR(Materials_Buildings_Alpha, 1.0f);
	CFGVAR(Materials_Buildings_IgnoreDepth, false);
	CFGVAR(Materials_Buildings_TwoModels, 0);
	CFGVAR(Materials_Buildings_OverlayAlpha, 0.5f);

	// Overlay
	CFGVAR(Materials_Buildings_Overlay, false);
	CFGVAR(Materials_Buildings_OverlayMaterial, 1);

	// Filtros
	CFGVAR(Materials_Buildings_Ignore_Local, false);
	CFGVAR(Materials_Buildings_Ignore_Teammates, false);
	CFGVAR(Materials_Buildings_Ignore_Enemies, false);
	CFGVAR(Materials_Buildings_Show_Teammate_Dispensers, false);

	// REMOVIDO: Materials_Buildings_HiddenMaterial
	// REMOVIDO: Materials_Buildings_TwoModels

	// ================= WORLD =================
	CFGVAR(Materials_World_Active, false);
	CFGVAR(Materials_World_Material, 0);
	CFGVAR(Materials_World_Alpha, 1.0f);
	CFGVAR(Materials_World_IgnoreDepth, false);

	// Overlay
	CFGVAR(Materials_World_Overlay, false);
	CFGVAR(Materials_World_OverlayMaterial, 1);
	CFGVAR(Materials_World_OverlayAlpha, 0.5f);

	// Filtros
	CFGVAR(Materials_World_Ignore_HealthPacks, false);
	CFGVAR(Materials_World_Ignore_AmmoPacks, false);
	CFGVAR(Materials_World_Ignore_Halloween_Gift, false);
	CFGVAR(Materials_World_Ignore_MVM_Money, false);
	CFGVAR(Materials_World_Ignore_LocalProjectiles, false);
	CFGVAR(Materials_World_Ignore_TeammateProjectiles, false);
	CFGVAR(Materials_World_Ignore_EnemyProjectiles, false);

	// REMOVIDO: Materials_World_HiddenMaterial
	// REMOVIDO: Materials_World_TwoModels

	// ================= HANDS =================
	CFGVAR(Materials_Hands_Active, false);
	CFGVAR(Materials_Hands_Material, 0);
	CFGVAR(Materials_Hands_Alpha, 1.0f);
	CFGVAR(Materials_Hands_IgnoreDepth, false);

	// Overlay
	CFGVAR(Materials_Hands_Overlay, false);
	CFGVAR(Materials_Hands_OverlayMaterial, 1);
	CFGVAR(Materials_Hands_OverlayAlpha, 0.5f);

	// REMOVIDO: Materials_Hands_HiddenMaterial
	// REMOVIDO: Materials_Hands_TwoModels
	// REMOVIDO: Materials_Hands_No_Depth (substituído por IgnoreDepth)

	// ================= WEAPONS =================
	CFGVAR(Materials_Weapons_Active, false);
	CFGVAR(Materials_Weapons_Material, 0);
	CFGVAR(Materials_Weapons_Alpha, 1.0f);
	CFGVAR(Materials_Weapons_IgnoreDepth, false);

	// Overlay
	CFGVAR(Materials_Weapons_Overlay, false);
	CFGVAR(Materials_Weapons_OverlayMaterial, 1);
	CFGVAR(Materials_Weapons_OverlayAlpha, 0.5f);

	// REMOVIDO: Materials_Weapons_HiddenMaterial
	// REMOVIDO: Materials_Weapons_TwoModels
	// REMOVIDO: Materials_Weapons_No_Depth (substituído por IgnoreDepth)

	// ================= VIEWMODEL =================
	CFGVAR(Materials_ViewModel_Active, false);
	CFGVAR(Materials_ViewModel_Hands_Material, 0);
	CFGVAR(Materials_ViewModel_Hands_Alpha, 1.0f);
	CFGVAR(Materials_ViewModel_Weapon_Material, 0);
	CFGVAR(Materials_ViewModel_Weapon_Alpha, 1.0f);
#pragma endregion
#pragma region Outlines
	CFGVAR(Outlines_Active, false);
	CFGVAR(Outlines_Style, 0);
	CFGVAR(Outlines_Bloom_Amount, 1.0f);
	CFGVAR(Outlines_Players_Alpha, 1.0f);
	CFGVAR(Outlines_Buildings_Alpha, 1.0f);
	CFGVAR(Outlines_World_Active, false);
	CFGVAR(Outlines_World_Alpha, 1.0f);
	CFGVAR(Outlines_World_Ignore_AmmoPacks, false);
	CFGVAR(Outlines_World_Ignore_EnemyProjectiles, false);
	CFGVAR(Outlines_World_Ignore_Halloween_Gift, false);
	CFGVAR(Outlines_World_Ignore_HealthPacks, false);
	CFGVAR(Outlines_World_Ignore_LocalProjectiles, false);
	CFGVAR(Outlines_World_Ignore_MVM_Money, false);
	CFGVAR(Outlines_World_Ignore_TeammateProjectiles, false);
	CFGVAR(Outlines_Players_Ignore_Friends, false);
	CFGVAR(Outlines_Buildings_Active, false);
	CFGVAR(Outlines_Buildings_Ignore_Enemies, false);
	CFGVAR(Outlines_Buildings_Ignore_Local, false);
	CFGVAR(Outlines_Buildings_Ignore_Teammates, false);
	CFGVAR(Outlines_Buildings_Show_Teammate_Dispensers, false);
	CFGVAR(Outlines_Players_Active, false);
	CFGVAR(Outlines_Players_Ignore_Enemies, false);
	CFGVAR(Outlines_Players_Ignore_Local, false);
	CFGVAR(Outlines_Players_Ignore_Teammates, false);
	CFGVAR(Outlines_Players_Show_Teammate_Medics, false);
#pragma endregion

#pragma region Misc
	CFGVAR(Misc_AutoJump, false);
	CFGVAR(Misc_AntiAFK_Enable, false);
	CFGVAR(Misc_Edge_Jump_Key, false);
	CFGVAR(Misc_SetupBones_Optimization, false);
	CFGVAR(Misc_Accuracy_Improvements, false);
	CFGVAR(Radio, false);
	CFGVAR(Misc_AutoStrafer_Enable, false);
	CFGVAR(Misc_AutoRocketJump_Enable, false);
	CFGVAR(Misc_AutoRocketJump_Key, 0);
	CFGVAR(Misc_AutoStrafer_Intensity, 45.f);
	CFGVAR(Radio_LocalMusic, false);
	CFGVAR(Misc_FakeLatencyfloat_Enable, 45.f);
	CFGVAR(Misc_FakeLatency_Enable, false);
	CFGVAR(ping_reducer, false);
	CFGVAR(Radio_Pause, false);
	CFGVAR(Radio_Next, false);
	CFGVAR(Radio_Prev, false);
	CFGVAR(Radio_VolUp, false);
	CFGVAR(Radio_VolDown, false);
	CFGVAR(Misc_AccuracyImprovements, false);
	CFGVAR(Misc_Fake_Taunt, false);
	CFGVAR(Misc_ThirdPerson_Distance, 0.f);
	CFGVAR(Misc_ThirdPerson_Enable, false);
	CFGVAR(Misc_ThirdPerson_Fov, 0.f);
	CFGVAR(Misc_ThirdPerson_Key, 0);
	CFGVAR(Misc_ThirdPerson_KeyMode, 0);
	CFGVAR(Misc_ThirdPerson_SideOffset, 0.f);
	CFGVAR(Misc_Clean_Screenshot, false);

	// ===== SEED PREDICTION / NO SPREAD =====
	CFGVAR(Exploits_SeedPred_Active, false);
	CFGVAR(Exploits_SeedPred_DrawIndicator, false);
#pragma endregion

#pragma region Shifting
	// ========== SHIFTING - SHIFT KEY (Controla o início do shift) ==========
	CFGVAR(shifting_active, false);							// Ativa/desativa o feature de shifting
	CFGVAR(shifting_key, -1);									// Tecla para iniciar o shift (default: sem tecla)
	CFGVAR(shifting_key_mode, 1);								// Modo da tecla: 0=Always, 1=Hold, 2=Toggle, 3=HoldOff
	CFGVAR(shifting_delay_ticks, 0.f);						// Delay em ticks antes de ativar
	CFGVAR(shifting_delay_hitscan, 0.f);					// Delay específico para hitscan
	CFGVAR(delay_hitscan, 0);									// Flag de delay para hitscan

	// ========== SHIFTING - RECHARGE KEY (Tecla para recarregar os ticks - CUSTOMIZÁVEL!) ==========
	CFGVAR(shifting_recharge_key, 'R');						// Tecla padrão: R (pode ser mudada!)
	CFGVAR(shifting_recharge_key_mode, 1);					// Modo: 0=Always, 1=Hold, 2=Toggle, 3=HoldOff

	// ========== SHIFTING - WARP KEY (Controla o warp - INDEPENDENTE!) ==========
	CFGVAR(shifting_warp, false);								// Ativa/desativa o warp
	CFGVAR(shifting_warp_key, -1);							// Tecla para ativar warp (pode ser DIFERENTE do shift!)
	CFGVAR(shifting_warp_key_mode, 1);						// Modo da tecla warp: 0=Always, 1=Hold, 2=Toggle, 3=HoldOff
#pragma endregion

#pragma region Colors
	CFGVAR(Color_Skeleton, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Skeleton_OnShot, Color_t(255, 255, 0, 255));
	CFGVAR(Color_Skeleton_OnHit, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Skeleton_Bounds, Color_t(0, 255, 255, 255));
	CFGVAR(Color_Skeleton_AimPoints, Color_t(255, 0, 0, 255));
	CFGVAR(Color_BacktrackSkeleton, Color_t(255, 255, 255, 100));
	CFGVAR(Color_TeamRed, Color_t(255, 0, 0, 255));
	CFGVAR(Color_TeamBlue, Color_t(0, 0, 255, 255));
	CFGVAR(Color_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Ammo, Color_t(255, 215, 0, 255));
	CFGVAR(Color_Medkit, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Flag, Color_t(255, 255, 0, 255));
	CFGVAR(Color_Uber, Color_t(255, 0, 255, 255));
	CFGVAR(Color_ESP_Outline, Color_t(0, 0, 0, 255));
	CFGVAR(Color_HealthBarBG, Color_t(0, 0, 0, 200));
	CFGVAR(Color_HealthLow, Color_t(255, 0, 0, 255));
	CFGVAR(Color_HealthHigh, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Overheal, Color_t(0, 0, 255, 255));
	CFGVAR(Color_CondsText, Color_t(255, 255, 255, 255));
	CFGVAR(Color_SniperLine, Color_t(255, 255, 255, 255));
	CFGVAR(Color_TracerLine, Color_t(255, 255, 255, 255));
	CFGVAR(Color_UberText, Color_t(255, 0, 255, 255));
	CFGVAR(Color_UberBar, Color_t(255, 0, 255, 255));
	CFGVAR(Color_UberOutline, Color_t(0, 0, 0, 255));
	CFGVAR(Color_AimbotFOV, Color_t(0, 0, 255, 255));
	CFGVAR(Color_ProjFOV, Color_t(0, 255, 0, 255));
	CFGVAR(Color_MeleeFOV, Color_t(255, 0, 0, 255));
	CFGVAR(Color_OffscreenArrow, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Name, Color_t(255, 255, 255, 255));
	CFGVAR(Color_HealthText, Color_t(255, 255, 255, 255));
	CFGVAR(Color_BuildingTeam, Color_t(0, 255, 0, 255));
	CFGVAR(Color_BuildingEnemy, Color_t(255, 0, 0, 255));
	CFGVAR(Color_BuildingName, Color_t(255, 255, 255, 255));
	CFGVAR(Color_AmmoPack, Color_t(255, 215, 0, 255));
	CFGVAR(Color_Cheater, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Enemy, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Friend, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Halloween_Gift, Color_t(255, 165, 0, 255));
	CFGVAR(Color_HealthPack, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Invisible, Color_t(128, 128, 128, 255));
	CFGVAR(Color_Invulnerable, Color_t(255, 215, 0, 255));
	CFGVAR(Color_MVM_Money, Color_t(0, 255, 255, 255));
	CFGVAR(Color_OverHeal, Color_t(0, 0, 255, 255));
	CFGVAR(Color_RetardLegit, Color_t(255, 255, 0, 255));
	CFGVAR(Color_Target, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Teammate, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Hands, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Hands_Sheen, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Props, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Weapon, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Weapon_Sheen, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Weapons, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Players_Friends, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Players_LagRecords, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Players_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Players_Overlay_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Players_Overlay_Friends, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Players_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Players_Overlay_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Players_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Players_Overlay_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Buildings_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Buildings_Overlay_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Buildings_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Buildings_Overlay_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Buildings_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Buildings_Overlay_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Color_HealthPack_Overlay, Color_t(0, 255, 0, 255));
	CFGVAR(Color_AmmoPack_Overlay, Color_t(255, 215, 0, 255));
	CFGVAR(Color_Halloween_Gift_Overlay, Color_t(255, 165, 0, 255));
	CFGVAR(Color_MVM_Money_Overlay, Color_t(0, 255, 255, 255));
	CFGVAR(Color_Projectiles_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Projectiles_Overlay_Local, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Projectiles_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Projectiles_Overlay_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Color_Projectiles_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Projectiles_Overlay_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Color_Hands_Overlay, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Weapons_Overlay, Color_t(255, 255, 255, 255));
	CFGVAR(Outlines_Color_Teammates, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_LocalPlayer, Color_t(255, 255, 255, 255));
	CFGVAR(Outlines_Color_Friends, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_TeammateMedics, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_Enemies, Color_t(255, 0, 0, 255));
	CFGVAR(Outlines_Color_LocalBuildings, Color_t(255, 255, 255, 255));
	CFGVAR(Outlines_Color_TeammateDispensers, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_TeammateBuildings, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_EnemyBuildings, Color_t(255, 0, 0, 255));
	CFGVAR(Outlines_Color_HealthPack, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_AmmoPack, Color_t(255, 215, 0, 255));
	CFGVAR(Outlines_Color_Halloween_Gift, Color_t(255, 165, 0, 255));
	CFGVAR(Outlines_Color_MVM_Money, Color_t(0, 255, 255, 255));
	CFGVAR(Outlines_Color_LocalProjectiles, Color_t(255, 255, 255, 255));
	CFGVAR(Outlines_Color_TeammateProjectiles, Color_t(0, 255, 0, 255));
	CFGVAR(Outlines_Color_EnemyProjectiles, Color_t(255, 0, 0, 255));
	CFGVAR(ESP_BoxColor, Color_t(255, 255, 255, 255));
	CFGVAR(ESP_NameColor, Color_t(255, 255, 255, 255));
	CFGVAR(ESP_TracerColor, Color_t(255, 255, 255, 255));
	CFGVAR(ESP_PingColor, Color_t(255, 255, 255, 255));
	CFGVAR(ESP_DistanceColor, Color_t(255, 255, 255, 255));
	CFGVAR(ESP_BuildColor, Color_t(255, 100, 100, 255));
	CFGVAR(ESP_PickupsColor, Color_t(100, 255, 100, 255));
	CFGVAR(ESP_PickupsBoxColor, Color_t(150, 150, 255, 255));
	CFGVAR(ESP_PickupsNameColor, Color_t(255, 255, 255, 255));
	CFGVAR(ESP_BoxCaptureColor, Color_t(255, 200, 0, 255));
	CFGVAR(ESP_NameCaptureColor, Color_t(255, 255, 100, 255));
	CFGVAR(ESP_OffscreenColor, Color_t(255, 50, 50, 255));
	CFGVAR(ESP_OffscreenFilledColor, Color_t(255, 100, 100, 180));
	CFGVAR(ESP_SniperLinesColor, Color_t(255, 0, 0, 200));
	CFGVAR(ESP_UberStatusColor, Color_t(0, 255, 255, 255));
	CFGVAR(ESP_UberBarColor, Color_t(0, 200, 255, 255));
	CFGVAR(Aimbot_FOVColor, Color_t(255, 255, 255, 100));
#pragma endregion

#pragma region Indicators
	CFGVAR(Indicators_Enable, false);
	CFGVAR(Indicators_Keybinds_Enable, false);
	CFGVAR(Indicators_Watermark_Enable, false);
	CFGVAR(Watermark_ShowName, false);
	CFGVAR(Watermark_ShowFPS, false);
	CFGVAR(Watermark_ShowTime, false);
	CFGVAR(Watermark_ShowPing, false);
	CFGVAR(Indicators_Show_FakeLatency, true);
	CFGVAR(Indicators_Show_RealLatency, true);
	CFGVAR(Indicators_Show_ScoreboardLatency, true);
	CFGVAR(Indicators_Show_Inaccuracy, true);
	CFGVAR(Indicators_Show_Velocity, true);
	CFGVAR(Indicators_Display_Mode, 0);
	CFGVAR(Indicators_Pos_X, 10.f);
	CFGVAR(Indicators_Pos_Y, 530.f);
	CFGVAR(Keybinds_Pos_X, 10.f);
	CFGVAR(Keybinds_Pos_Y, 624.f);
	CFGVAR(Watermark_Pos_X, 10.f);
	CFGVAR(Watermark_Pos_Y, 10.f);
	CFGVAR(Spectators_Pos_X, 10.f);
	CFGVAR(Spectators_Pos_Y, 718.f);
#pragma endregion

	CFGVAR(CurrentSection, 0);
	CFGVAR(Menu_ThemeColor, Color_t(0, 122, 187, 255));
	CFGVAR(Menu_ModifyTheme, false);
}