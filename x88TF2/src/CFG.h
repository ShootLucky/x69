#pragma once
#include "Utils/Config/Config.h"
namespace CFG
{
#pragma region Aimbot
	CFGVAR(Aimbot_Enable, false);
	// 0 = hold (use key while held or mouse click), 1 = toggle (press to toggle), 2 = always on
	CFGVAR(Aimbot_Key, 0);
	CFGVAR(Aimbot_KeyMode, 0);
	CFGVAR(Aimbot_FOV, 10.f);
	CFGVAR(Aimbot_VisibleCheck, true);
	CFGVAR(Aimbot_TeamCheck, true);
	CFGVAR(Aimbot_SilentAim, false);
	CFGVAR(Aimbot_Aimlock, false);
	CFGVAR(Aimbot_Hitscan_Smoothing, 10.f);
	CFGVAR(Aimbot_Hitbox_Head, false);
	CFGVAR(Aimbot_Hitbox_Neck, false);
	CFGVAR(Aimbot_Hitbox_Chest, false);
	CFGVAR(Aimbot_Hitbox_Pelvis, false);
#pragma endregion
#pragma region ESP
	CFGVAR(ESP_Enable, false);
	CFGVAR(ESP_Box, false);
	CFGVAR(ESP_Name, false);
	CFGVAR(ESP_Health, false);
	CFGVAR(ESP_Team, false);
	// Novas opções para Skeleton ESP e Chams Box
	CFGVAR(ESP_Skeleton, false);
	CFGVAR(ESP_ChamsBox, false);
	CFGVAR(ESP_ChamsTeam, false);
	CFGVAR(ESP_CaptureFlag, false);
	CFGVAR(ESP_Build, false);
	CFGVAR(ESP_BuildOnlyEnemy, false);
	CFGVAR(ESP_ChamsBuild, false);
	CFGVAR(ESP_ChamsBuildOnlyEnemy, false);
	CFGVAR(ESP_ChamsHideCloaked, false);
	CFGVAR(ESP_ChamsLocalPlayer, false);
	CFGVAR(ESP_HideCloaked, false);
	CFGVAR(ESP_LocalPlayer, false);
	CFGVAR(ESP_Offscreen, false);
	CFGVAR(ESP_Pickups, false);
	CFGVAR(ESP_PickupsBox, false);
	CFGVAR(ESP_PickupsName, false);
	CFGVAR(ESP_SkeletonBuild, false);
	CFGVAR(ESP_SkeletonBuildOnlyEnemy, false);
	CFGVAR(ESP_SkeletonHideCloaked, false);
	CFGVAR(ESP_SkeletonLocalPlayer, false);
	CFGVAR(ESP_SkeletonTeam, false);
	// Radio options
	CFGVAR(Radio, false);
	CFGVAR(Radio_LocalMusic, false);
	CFGVAR(Radio_Internacional, false);
	CFGVAR(Radio_Country, 0);
	// Bullet Tracer options
	CFGVAR(BulletTracer, false);
	CFGVAR(BulletTracer_Type, 0); // 0=line, 1=line+box, 2=box
	CFGVAR(ESP_BoxCapture, false);
	CFGVAR(ESP_NameCapture, false);
	CFGVAR(ESP_ChamsCaptureFlag, false);
	CFGVAR(ESP_SkeletonCaptureFlag, false);
#pragma endregion
#pragma region Misc
	CFGVAR(Misc_AutoJump, true);
#pragma endregion
#pragma region TEST
	CFGVAR(Important_checkbox, true);
	CFGVAR(TestCombo, 0);
	CFGVAR(TestFloat, 0.f);
	CFGVAR(TestInt, 0);
#pragma endregion
#pragma region Colors
#pragma endregion
	CFGVAR(CurrentSection, 0);
}