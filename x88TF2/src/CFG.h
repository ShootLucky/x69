#pragma once
#include "Utils/Config/Config.h"
namespace CFG
{
#pragma region Aimbot
	CFGVAR(Aimbot_Enable, false);
	CFGVAR(Aimbot_Key, 0);
	CFGVAR(Aimbot_KeyMode, 0);
	CFGVAR(Aimbot_Melee_KeyMode, 0);
	CFGVAR(Aimbot_Projectile_KeyMode, 0);
	CFGVAR(Aimbot_FOV, 45.f);
	CFGVAR(Aimbot_VisibleCheck, true);
	CFGVAR(Aimbot_TeamCheck, true);
	CFGVAR(Aimbot_Hitscan_Smoothing, 5.f);
	CFGVAR(Aimbot_Hitbox_Head, false);
	CFGVAR(Aimbot_Hitbox_Neck, false);
	CFGVAR(Aimbot_Hitbox_Chest, false);
	CFGVAR(Aimbot_Hitbox_Pelvis, false);
	CFGVAR(Aimbot_Hitbox_Body, false);
	CFGVAR(Aimbot_Hitbox_Buildings, false);
	CFGVAR(Aimbot_Projectile_Enable, false);
	CFGVAR(Aimbot_Projectile_Key, 0); //Aimbot_Hitscan_Multipoint_Scale
	CFGVAR(Aimbot_Hitscan_Multipoint_Scale, 0.f);
	CFGVAR(Aimbot_Projectile_FOV, 45.f);
	CFGVAR(Aimbot_Projectile_Smoothing, 5.f);
	CFGVAR(Aimbot_Projectile_TicksPredict, 0);
	CFGVAR(Aimbot_Projectile_TeamCheck, true);
	CFGVAR(Debug_SplashPoints, false);
	CFGVAR(Aimbot_Projectile_Mode, 0);
	CFGVAR(Aimbot_ActiveShoot, false);
	CFGVAR(Aimbot_ActiveLagRecords, false);
	CFGVAR(Aimbot_AutoShoot, false);
	CFGVAR(Aimbot_TargetStickies, false);
	CFGVAR(Aimbot_SmoothAutoShoot, false);
	CFGVAR(Aimbot_WaitForHeadshot, false);
	CFGVAR(Aimbot_AutoScope, false);
	CFGVAR(Aimbot_MinigunTapfire, false);
	CFGVAR(Aimbot_WaitForCharge, false)
		CFGVAR(Aimbot_Target_Players, true);
	CFGVAR(Aimbot_Target_Buildings, false);
	CFGVAR(Aimbot_Ignore_Friends, true);
	CFGVAR(Aimbot_Ignore_Invulnerable, true);
	CFGVAR(Aimbot_ActiveMelee, false);
	CFGVAR(Aimbot_Hitscan_Mode, 0);
	CFGVAR(Aimbot_Hitscan_Sort, 0);
	CFGVAR(Aimbot_TargetLagRecords, false);
	CFGVAR(Aimbot_WalkToTarget, false);
	CFGVAR(Aimbot_WhitelistTeammates, false);
	CFGVAR(Aimbot_BaimAfterShots, 0);
	CFGVAR(Aimbot_BaimAfterHealth, 0.18f);
	CFGVAR(Aimbot_Hitbox_Sort, 0);
	CFGVAR(Aimbot_Projectile_NoSpread, false);//Aimbot_Projectile_AimPosition Aimbot_Projectile_BBoxMultipoint Aimbot_Hitscan_AlwaysActive
	CFGVAR(Aimbot_Projectile_AutoDoubleDonk, false); //Misc_Accuracy_Improvements
	CFGVAR(Aimbot_Projectile_AimPosition, 0);
	CFGVAR(Aimbot_Projectile_GroundStrafePrediction, false);
	CFGVAR(Aimbot_Projectile_AdvancedAirStrafe, false);
	CFGVAR(Aimbot_Projectile_RocketSplashPoint, false);
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
	CFGVAR(Aimbot_Ignore_Invisible, true);
	CFGVAR(Aimbot_Ignore_Taunting, true);
	CFGVAR(Aimbot_Projectile_Active, false);
	CFGVAR(Aimbot_Projectile_Advanced_Head_Aim, false);
	CFGVAR(Aimbot_Projectile_Auto_Double_Donk, false);
	CFGVAR(Aimbot_Projectile_BBox_Multipoint, false);
	CFGVAR(Aimbot_Projectile_Max_Processing_Targets, 5);
	CFGVAR(Aimbot_Projectile_Max_Simulation_Time, 2.0f);
	CFGVAR(Aimbot_Projectile_Rocket_Splash, false);
	CFGVAR(Aimbot_Hitscan_Hitbox, 0);
	CFGVAR(Aimbot_Hitscan_Scan_Head, true);
	CFGVAR(Aimbot_Hitscan_Scan_Body, true);
	CFGVAR(Aimbot_Hitscan_Scan_Arms, false);
	CFGVAR(Aimbot_Hitscan_Scan_Legs, false);
	CFGVAR(Aimbot_Hitscan_Scan_Buildings, true);
	CFGVAR(Aimbot_Projectile_SplashBot, false);
	CFGVAR(Aimbot_Projectile_SplashPoints, 80.0f);
#pragma endregion
#pragma region ESP
	CFGVAR(ESP_Enable, false);
	CFGVAR(ESP_Box, false);//ESP_BoxType
	CFGVAR(ESP_BoxType, 0);
	CFGVAR(ESP_Name, false);
	CFGVAR(ESP_Health, false);
	CFGVAR(ESP_Team, false);
	// Novas opções para Skeleton ESP e Chams Box
	CFGVAR(ESP_Skeleton, false);
	CFGVAR(ESP_CaptureFlag, false);
	CFGVAR(ESP_Build, false);
	CFGVAR(ESP_BuildOnlyEnemy, false);
	CFGVAR(ESP_HideCloaked, false);
	CFGVAR(ESP_LocalPlayer, false);
	CFGVAR(ESP_Offscreen, false);
	CFGVAR(ESP_Pickups, false);
	CFGVAR(ESP_PickupsBox, false);
	CFGVAR(ESP_PickupsName, false);
	CFGVAR(Aimbot_DrawFOV, false);
	CFGVAR(ESP_SkeletonBuild, false);
	CFGVAR(ESP_SkeletonBuildOnlyEnemy, false);
	CFGVAR(ESP_SkeletonHideCloaked, false);
	CFGVAR(ESP_SkeletonLocalPlayer, false);
	CFGVAR(ESP_SkeletonTeam, false);
	CFGVAR(ESP_SkeletonCaptureFlag, false);
	// Bullet Tracer options
	CFGVAR(BulletTracer, false);
	CFGVAR(BulletTracer_Type, 0); // 0=line, 1=line+box, 2=box
	CFGVAR(BulletTracer_Width, 1.0f);
	CFGVAR(BulletTracer_Speed, 0.0f);
	CFGVAR(BulletTracer_Length, 3.0f);
	CFGVAR(ESP_BoxCapture, false);
	CFGVAR(ESP_NameCapture, false);
	CFGVAR(Visuals_Draw_Movement_Path_Style, 0);
	CFGVAR(ESP_HealthType, 0);
	CFGVAR(ESP_Skeleton_Backtrack, false);
	CFGVAR(ESP_Skeleton_BacktrackType, 0);
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
	CFGVAR(Visuals_Rain_Width, 1.f);
	CFGVAR(Visuals_Rain_Length, 20.f);
	CFGVAR(Visuals_Rain_Radius, 1000.f);
	CFGVAR(Visuals_Rain_WindDirection, 0.f);
	CFGVAR(Visuals_Rain_WindSpeed, 10.f);
	CFGVAR(ESP_Arrows, false);
	CFGVAR(ESP_Conds, false);
	CFGVAR(ESP_SniperLines, false);
	CFGVAR(ESP_Tracer, false);
	CFGVAR(ESP_Uber, false);
	CFGVAR(ESP_UberBar, false);
#pragma endregion
#pragma region Misc
	CFGVAR(Misc_AutoJump, true); //Misc_SetupBones_Optimization //Misc_Edge_Jump_Key Misc_AntiAFK_Enable
	CFGVAR(Misc_AntiAFK_Enable, true);
	CFGVAR(Misc_Edge_Jump_Key, true);
	CFGVAR(Misc_SetupBones_Optimization, false);
	CFGVAR(Misc_Accuracy_Improvements, false);
	CFGVAR(Radio, false)
		CFGVAR(Misc_AutoStrafer_Enable, false);
	CFGVAR(Misc_AutoRocketJump_Enable, false);
	CFGVAR(Misc_AutoRocketJump_Key, 0);
	CFGVAR(Misc_AutoStrafer_Intensity, 45.f);
	CFGVAR(Radio_LocalMusic, false);
	CFGVAR(Misc_FakeLatencyfloat_Enable, 45.f);
	CFGVAR(Misc_FakeLatency_Enable, false);
	CFGVAR(Radio_Pause, false);
	CFGVAR(Radio_Next, false);
	CFGVAR(Radio_Prev, false);
	CFGVAR(Radio_VolUp, false);
	CFGVAR(Radio_VolDown, false);
	CFGVAR(Misc_AccuracyImprovements, true);
	CFGVAR(Misc_Fake_Taunt, false);
	CFGVAR(Misc_ThirdPerson_Distance, 0.f);
	CFGVAR(Misc_ThirdPerson_Enable, false);
	CFGVAR(Misc_ThirdPerson_Fov, 0.f);
	CFGVAR(Misc_ThirdPerson_Key, 0);
	CFGVAR(Misc_ThirdPerson_KeyMode, 0);
	CFGVAR(Misc_ThirdPerson_SideOffset, 0.f);
#pragma endregion
#pragma region Colors
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
	CFGVAR(Color_BacktrackSkeleton, Color_t(255, 255, 255, 255));
	CFGVAR(Color_AimbotFOV, Color_t(0, 0, 255, 255));
	CFGVAR(Color_ProjFOV, Color_t(0, 255, 0, 255));
	CFGVAR(Color_MeleeFOV, Color_t(255, 0, 0, 255));
	CFGVAR(Color_OffscreenArrow, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Name, Color_t(255, 255, 255, 255));
	CFGVAR(Color_HealthText, Color_t(255, 255, 255, 255));
	CFGVAR(Color_Skeleton, Color_t(255, 255, 255, 255));
	CFGVAR(Color_BuildingTeam, Color_t(0, 255, 0, 255));
	CFGVAR(Color_BuildingEnemy, Color_t(255, 0, 0, 255));
	CFGVAR(Color_BuildingName, Color_t(255, 255, 255, 255));
#pragma endregion
	CFGVAR(CurrentSection, 0);
}