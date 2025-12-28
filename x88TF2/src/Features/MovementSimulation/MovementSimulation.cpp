// MovementSimulation.cpp
#include "MovementSimulation.h"

#include "../LagRecords/Backtrack.h"

#include "CFG.h"

void CMovementSimulation::CPlayerDataBackup::Store(C_TFPlayer* pPlayer)
{
	m_vecOrigin = pPlayer->m_vecOrigin();
	m_vecVelocity = pPlayer->m_vecVelocity();
	m_vecBaseVelocity = pPlayer->m_vecBaseVelocity();
	m_vecViewOffset = pPlayer->m_vecViewOffset();
	m_hGroundEntity = pPlayer->m_hGroundEntity();
	m_fFlags = pPlayer->m_fFlags();
	m_flDucktime = pPlayer->m_flDucktime();
	m_flDuckJumpTime = pPlayer->m_flDuckJumpTime();
	m_bDucked = pPlayer->m_bDucked();
	m_bDucking = pPlayer->m_bDucking();
	m_bInDuckJump = pPlayer->m_bInDuckJump();
	m_flModelScale = pPlayer->m_flModelScale();
	m_nButtons = pPlayer->m_nButtons();
	m_flLastMovementStunChange = pPlayer->m_flLastMovementStunChange();
	m_flStunLerpTarget = pPlayer->m_flStunLerpTarget();
	m_bStunNeedsFadeOut = pPlayer->m_bStunNeedsFadeOut();
	m_flPrevTauntYaw = pPlayer->m_flPrevTauntYaw();
	m_flTauntYaw = pPlayer->m_flTauntYaw();
	m_flCurrentTauntMoveSpeed = pPlayer->m_flCurrentTauntMoveSpeed();
	m_iKartState = pPlayer->m_iKartState();
	m_flVehicleReverseTime = pPlayer->m_flVehicleReverseTime();
	m_flHypeMeter = pPlayer->m_flHypeMeter();
	m_flMaxspeed = pPlayer->m_flMaxspeed();
	m_nAirDucked = pPlayer->m_nAirDucked();
	m_bJumping = pPlayer->m_bJumping();
	m_iAirDash = pPlayer->m_iAirDash();
	m_flWaterJumpTime = pPlayer->m_flWaterJumpTime();
	m_flSwimSoundTime = pPlayer->m_flSwimSoundTime();
	m_surfaceProps = pPlayer->m_surfaceProps();
	m_pSurfaceData = pPlayer->m_pSurfaceData();
	m_surfaceFriction = pPlayer->m_surfaceFriction();
	m_chTextureType = pPlayer->m_chTextureType();
	m_vecPunchAngle = pPlayer->m_vecPunchAngle();
	m_vecPunchAngleVel = pPlayer->m_vecPunchAngleVel();
	m_flJumpTime = pPlayer->m_flJumpTime();
	m_MoveType = pPlayer->m_MoveType();
	m_MoveCollide = pPlayer->m_MoveCollide();
	m_vecLadderNormal = pPlayer->m_vecLadderNormal();
	m_flGravity = pPlayer->m_flGravity();
	m_nWaterLevel = pPlayer->m_nWaterLevel_C_BaseEntity();
	m_nWaterType = pPlayer->m_nWaterType();
	m_flFallVelocity = pPlayer->m_flFallVelocity();
	m_nPlayerCond = pPlayer->m_nPlayerCond();
	m_nPlayerCondEx = pPlayer->m_nPlayerCondEx();
	m_nPlayerCondEx2 = pPlayer->m_nPlayerCondEx2();
	m_nPlayerCondEx3 = pPlayer->m_nPlayerCondEx3();
	m_nPlayerCondEx4 = pPlayer->m_nPlayerCondEx4();
	_condition_bits = pPlayer->_condition_bits();
}

void CMovementSimulation::CPlayerDataBackup::Restore(C_TFPlayer* pPlayer)
{
	pPlayer->m_vecOrigin() = m_vecOrigin;
	pPlayer->m_vecVelocity() = m_vecVelocity;
	pPlayer->m_vecBaseVelocity() = m_vecBaseVelocity;
	pPlayer->m_vecViewOffset() = m_vecViewOffset;
	pPlayer->m_hGroundEntity() = m_hGroundEntity;
	pPlayer->m_fFlags() = m_fFlags;
	pPlayer->m_flDucktime() = m_flDucktime;
	pPlayer->m_flDuckJumpTime() = m_flDuckJumpTime;
	pPlayer->m_bDucked() = m_bDucked;
	pPlayer->m_bDucking() = m_bDucking;
	pPlayer->m_bInDuckJump() = m_bInDuckJump;
	pPlayer->m_flModelScale() = m_flModelScale;
	pPlayer->m_nButtons() = m_nButtons;
	pPlayer->m_flLastMovementStunChange() = m_flLastMovementStunChange;
	pPlayer->m_flStunLerpTarget() = m_flStunLerpTarget;
	pPlayer->m_bStunNeedsFadeOut() = m_bStunNeedsFadeOut;
	pPlayer->m_flPrevTauntYaw() = m_flPrevTauntYaw;
	pPlayer->m_flTauntYaw() = m_flTauntYaw;
	pPlayer->m_flCurrentTauntMoveSpeed() = m_flCurrentTauntMoveSpeed;
	pPlayer->m_iKartState() = m_iKartState;
	pPlayer->m_flVehicleReverseTime() = m_flVehicleReverseTime;
	pPlayer->m_flHypeMeter() = m_flHypeMeter;
	pPlayer->m_flMaxspeed() = m_flMaxspeed;
	pPlayer->m_nAirDucked() = m_nAirDucked;
	pPlayer->m_bJumping() = m_bJumping;
	pPlayer->m_iAirDash() = m_iAirDash;
	pPlayer->m_flWaterJumpTime() = m_flWaterJumpTime;
	pPlayer->m_flSwimSoundTime() = m_flSwimSoundTime;
	pPlayer->m_surfaceProps() = m_surfaceProps;
	pPlayer->m_pSurfaceData() = m_pSurfaceData;
	pPlayer->m_surfaceFriction() = m_surfaceFriction;
	pPlayer->m_chTextureType() = m_chTextureType;
	pPlayer->m_vecPunchAngle() = m_vecPunchAngle;
	pPlayer->m_vecPunchAngleVel() = m_vecPunchAngleVel;
	pPlayer->m_flJumpTime() = m_flJumpTime;
	pPlayer->m_MoveType() = m_MoveType;
	pPlayer->m_MoveCollide() = m_MoveCollide;
	pPlayer->m_vecLadderNormal() = m_vecLadderNormal;
	pPlayer->m_flGravity() = m_flGravity;
	pPlayer->m_nWaterLevel_C_BaseEntity() = m_nWaterLevel;
	pPlayer->m_nWaterType() = m_nWaterType;
	pPlayer->m_flFallVelocity() = m_flFallVelocity;
	pPlayer->m_nPlayerCond() = m_nPlayerCond;
	pPlayer->m_nPlayerCondEx() = m_nPlayerCondEx;
	pPlayer->m_nPlayerCondEx2() = m_nPlayerCondEx2;
	pPlayer->m_nPlayerCondEx3() = m_nPlayerCondEx3;
	pPlayer->m_nPlayerCondEx4() = m_nPlayerCondEx4;
	pPlayer->_condition_bits() = _condition_bits;
}

void CMovementSimulation::CPlayerDataCurrent::UpdateFromPlayer(C_TFPlayer* pPlayer)
{
	m_vecOrigin = pPlayer->m_vecOrigin();
	m_vecVelocity = pPlayer->m_vecVelocity();
	m_vecBaseVelocity = pPlayer->m_vecBaseVelocity();
	m_vecViewOffset = pPlayer->m_vecViewOffset();
	m_hGroundEntity = pPlayer->m_hGroundEntity();
	m_fFlags = pPlayer->m_fFlags();
	m_flDucktime = pPlayer->m_flDucktime();
	m_flDuckJumpTime = pPlayer->m_flDuckJumpTime();
	m_bDucked = pPlayer->m_bDucked();
	m_bDucking = pPlayer->m_bDucking();
	m_bInDuckJump = pPlayer->m_bInDuckJump();
	m_flModelScale = pPlayer->m_flModelScale();
	m_nButtons = pPlayer->m_nButtons();
	m_flLastMovementStunChange = pPlayer->m_flLastMovementStunChange();
	m_flStunLerpTarget = pPlayer->m_flStunLerpTarget();
	m_bStunNeedsFadeOut = pPlayer->m_bStunNeedsFadeOut();
	m_flPrevTauntYaw = pPlayer->m_flPrevTauntYaw();
	m_flTauntYaw = pPlayer->m_flTauntYaw();
	m_flCurrentTauntMoveSpeed = pPlayer->m_flCurrentTauntMoveSpeed();
	m_iKartState = pPlayer->m_iKartState();
	m_flVehicleReverseTime = pPlayer->m_flVehicleReverseTime();
	m_flHypeMeter = pPlayer->m_flHypeMeter();
	m_flMaxspeed = pPlayer->m_flMaxspeed();
	m_nAirDucked = pPlayer->m_nAirDucked();
	m_bJumping = pPlayer->m_bJumping();
	m_iAirDash = pPlayer->m_iAirDash();
	m_flWaterJumpTime = pPlayer->m_flWaterJumpTime();
	m_flSwimSoundTime = pPlayer->m_flSwimSoundTime();
	m_surfaceProps = pPlayer->m_surfaceProps();
	m_pSurfaceData = pPlayer->m_pSurfaceData();
	m_surfaceFriction = pPlayer->m_surfaceFriction();
	m_chTextureType = pPlayer->m_chTextureType();
	m_vecPunchAngle = pPlayer->m_vecPunchAngle();
	m_vecPunchAngleVel = pPlayer->m_vecPunchAngleVel();
	m_flJumpTime = pPlayer->m_flJumpTime();
	m_MoveType = pPlayer->m_MoveType();
	m_MoveCollide = pPlayer->m_MoveCollide();
	m_vecLadderNormal = pPlayer->m_vecLadderNormal();
	m_flGravity = pPlayer->m_flGravity();
	m_nWaterLevel = pPlayer->m_nWaterLevel_C_BaseEntity();
	m_nWaterType = pPlayer->m_nWaterType();
	m_flFallVelocity = pPlayer->m_flFallVelocity();
	m_nPlayerCond = pPlayer->m_nPlayerCond();
	m_nPlayerCondEx = pPlayer->m_nPlayerCondEx();
	m_nPlayerCondEx2 = pPlayer->m_nPlayerCondEx2();
	m_nPlayerCondEx3 = pPlayer->m_nPlayerCondEx3();
	m_nPlayerCondEx4 = pPlayer->m_nPlayerCondEx4();
	_condition_bits = pPlayer->_condition_bits();
}

void CMovementSimulation::SetupMoveData(C_TFPlayer* pPlayer, CMoveData* pMoveData)
{
	if (!pPlayer || !pMoveData)
		return;

	pMoveData->m_bFirstRunOfFunctions = false;
	pMoveData->m_bGameCodeMovedPlayer = false;
	pMoveData->m_nPlayerHandle = pPlayer->GetRefEHandle();
	pMoveData->m_vecVelocity = pPlayer->m_vecVelocity();
	pMoveData->m_vecAbsOrigin = pPlayer->m_vecOrigin();
	pMoveData->m_flMaxSpeed = pPlayer->TeamFortress_CalculateMaxSpeed();

	if (m_PlayerDataCurrent.m_fFlags & FL_DUCKING)
		pMoveData->m_flMaxSpeed *= 0.3333f;

	pMoveData->m_flClientMaxSpeed = pMoveData->m_flMaxSpeed;

	float yaw = 0.0f;
	if (F::LagRecords->HasRecords(pPlayer))
	{
		auto rec = F::LagRecords->GetRecord(pPlayer, 0);
		if (rec)
		{
			yaw = rec->EyeAngles.y;
		}
	}
	else
	{
		yaw = Math::VelocityToAngles(pMoveData->m_vecVelocity).y;
	}
	pMoveData->m_vecViewAngles = { 0.0f, yaw, 0.0f };

	Vec3 vForward = {}, vRight = {};
	Math::AngleVectors(pMoveData->m_vecViewAngles, &vForward, &vRight, nullptr);

	if (CFG::Aimbot_Projectile_PredictionMethod == 0)
	{
		pMoveData->m_flForwardMove = 450.0f;
		pMoveData->m_flSideMove = 0.0f;
	}
	else
	{
		pMoveData->m_flForwardMove = pMoveData->m_vecVelocity.Dot(vForward);
		pMoveData->m_flSideMove = pMoveData->m_vecVelocity.Dot(vRight);
	}

	const float flSpeed = pPlayer->m_vecVelocity().Length2D();

	if (flSpeed <= pMoveData->m_flMaxSpeed * 0.1f)
		pMoveData->m_flForwardMove = pMoveData->m_flSideMove = 0.0f;

	pMoveData->m_vecAngles = pMoveData->m_vecViewAngles;
	pMoveData->m_vecOldAngles = pMoveData->m_vecAngles;

	if (pPlayer->m_hConstraintEntity())
		pMoveData->m_vecConstraintCenter = pPlayer->m_hConstraintEntity()->GetAbsOrigin();

	else pMoveData->m_vecConstraintCenter = pPlayer->m_vecConstraintCenter();

	pMoveData->m_flConstraintRadius = pPlayer->m_flConstraintRadius();
	pMoveData->m_flConstraintWidth = pPlayer->m_flConstraintWidth();
	pMoveData->m_flConstraintSpeedFactor = pPlayer->m_flConstraintSpeedFactor();

	m_flYawTurnRate = 0.0f;

	// Ground strafe prediction - further enhanced with Amalgam-inspired improvements for accuracy
	if (CFG::Aimbot_Projectile_GroundStrafePrediction && (m_PlayerDataCurrent.m_fFlags & FL_ONGROUND) && F::LagRecords->HasRecords(pPlayer))
	{
		// Lowered min speed threshold for better detection of slower strafes
		const float flMinSpeed = m_MoveData.m_flMaxSpeed * 0.3f;

		// Increased to 10 samples for more robust detection and smoother averaging
		const auto pRecord0 = F::LagRecords->GetRecord(pPlayer, 0);
		const auto pRecord1 = F::LagRecords->GetRecord(pPlayer, 1);
		const auto pRecord2 = F::LagRecords->GetRecord(pPlayer, 2);
		const auto pRecord3 = F::LagRecords->GetRecord(pPlayer, 3);
		const auto pRecord4 = F::LagRecords->GetRecord(pPlayer, 4);
		const auto pRecord5 = F::LagRecords->GetRecord(pPlayer, 5);
		const auto pRecord6 = F::LagRecords->GetRecord(pPlayer, 6);
		const auto pRecord7 = F::LagRecords->GetRecord(pPlayer, 7);
		const auto pRecord8 = F::LagRecords->GetRecord(pPlayer, 8);
		const auto pRecord9 = F::LagRecords->GetRecord(pPlayer, 9);

		if (pRecord0 && pRecord1 && pRecord2 && pRecord3 && pRecord4 && pRecord5 && pRecord6 && pRecord7 && pRecord8 && pRecord9)
		{
			if (!(pRecord0->Flags & FL_ONGROUND) || !(pRecord1->Flags & FL_ONGROUND) || !(pRecord2->Flags & FL_ONGROUND) ||
				!(pRecord3->Flags & FL_ONGROUND) || !(pRecord4->Flags & FL_ONGROUND) || !(pRecord5->Flags & FL_ONGROUND) ||
				!(pRecord6->Flags & FL_ONGROUND) || !(pRecord7->Flags & FL_ONGROUND) || !(pRecord8->Flags & FL_ONGROUND) ||
				!(pRecord9->Flags & FL_ONGROUND))
			{
				return;
			}

			auto GetNormal = [&](const LagRecord_t* rec) -> Vec3
				{
					Vec3 start = rec->AbsOrigin;
					start.z += 1.0f;
					Vec3 end = rec->AbsOrigin;
					end.z -= 32.0f;

					trace_t trace{};
					Ray_t ray;
					ray.Init(start, end);
					CTraceFilterSimple filter(pPlayer, COLLISION_GROUP_NONE);
					I::EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &filter, &trace);

					if (trace.fraction < 1.0f && !trace.startsolid)
					{
						return trace.plane.normal;
					}

					return { 0.0f, 0.0f, 1.0f };
				};

			// Vector-based rotation calculation (Amalgam/jvnkbinv1 inspired)
			auto CalculateRotationAngle = [](const Vec3& v1, const Vec3& v2, const Vec3& n) -> float
				{
					Vec3 v1_proj = v1 - n * v1.Dot(n);
					Vec3 v2_proj = v2 - n * v2.Dot(n);

					float len1 = v1_proj.Length();
					float len2 = v2_proj.Length();

					if (len1 < 0.01f || len2 < 0.01f)
						return 0.0f;

					v1_proj.Normalize();
					v2_proj.Normalize();

					float cos_theta = v1_proj.Dot(v2_proj);
					Vec3 cross = v1_proj.Cross(v2_proj);
					float sin_theta = cross.Length() * (cross.Dot(n) >= 0.0f ? 1.0f : -1.0f);

					return atan2f(sin_theta, cos_theta) * (180.0f / PI);
				};

			Vec3 current_normal = GetNormal(pRecord0);
			Vec3 current_vel_proj = m_MoveData.m_vecVelocity - current_normal * (m_MoveData.m_vecVelocity.Dot(current_normal));
			if (current_vel_proj.Length() < flMinSpeed)
			{
				return;
			}

			// Calculate more rotation deltas
			const float flDelta0 = CalculateRotationAngle(pRecord1->Velocity, pRecord0->Velocity, GetNormal(pRecord0));
			const float flDelta1 = CalculateRotationAngle(pRecord2->Velocity, pRecord1->Velocity, GetNormal(pRecord1));
			const float flDelta2 = CalculateRotationAngle(pRecord3->Velocity, pRecord2->Velocity, GetNormal(pRecord2));
			const float flDelta3 = CalculateRotationAngle(pRecord4->Velocity, pRecord3->Velocity, GetNormal(pRecord3));
			const float flDelta4 = CalculateRotationAngle(pRecord5->Velocity, pRecord4->Velocity, GetNormal(pRecord4));
			const float flDelta5 = CalculateRotationAngle(pRecord6->Velocity, pRecord5->Velocity, GetNormal(pRecord5));
			const float flDelta6 = CalculateRotationAngle(pRecord7->Velocity, pRecord6->Velocity, GetNormal(pRecord6));
			const float flDelta7 = CalculateRotationAngle(pRecord8->Velocity, pRecord7->Velocity, GetNormal(pRecord7));
			const float flDelta8 = CalculateRotationAngle(pRecord9->Velocity, pRecord8->Velocity, GetNormal(pRecord8));

			// Stricter rejection for large deltas on ground (reduced from 45 to 35 for accuracy)
			if (fabsf(flDelta0) > 35.0f || fabsf(flDelta1) > 35.0f || fabsf(flDelta2) > 35.0f ||
				fabsf(flDelta3) > 35.0f || fabsf(flDelta4) > 35.0f || fabsf(flDelta5) > 35.0f ||
				fabsf(flDelta6) > 35.0f || fabsf(flDelta7) > 35.0f || fabsf(flDelta8) > 35.0f)
			{
				return;
			}

			// Stricter direction change limit (reduced to 1 for ground consistency)
			int iDirectionChanges = 0;
			int iLastSign = flDelta0 > 0.0f ? 1 : (flDelta0 < 0.0f ? -1 : 0);

			const float deltas[] = { flDelta1, flDelta2, flDelta3, flDelta4, flDelta5, flDelta6, flDelta7, flDelta8 };
			for (const float delta : deltas)
			{
				const int iCurrSign = delta > 0.0f ? 1 : (delta < 0.0f ? -1 : 0);
				if (iCurrSign != 0 && iLastSign != 0 && iCurrSign != iLastSign)
				{
					iDirectionChanges++;
				}
				if (iCurrSign != 0)
				{
					iLastSign = iCurrSign;
				}
			}

			if (iDirectionChanges > 1)
			{
				return;
			}

			// Adjusted weighted average with more emphasis on recent samples
			const float flWeightedAvg = (flDelta0 * 4.0f + flDelta1 * 3.5f + flDelta2 * 3.0f +
				flDelta3 * 2.5f + flDelta4 * 2.0f + flDelta5 * 1.5f + flDelta6 * 1.2f +
				flDelta7 * 1.0f + flDelta8 * 0.8f) / 19.5f;

			// Lowered threshold for better sensitivity
			if (fabsf(flWeightedAvg) < 0.35f)
			{
				return;
			}

			// Improved speed ratio clamping for better low-speed accuracy
			const float flSpeedRatio = std::clamp(current_vel_proj.Length() / m_MoveData.m_flMaxSpeed, 0.4f, 1.0f);

			// Adjusted clamp range for ground
			m_flYawTurnRate = std::clamp(flWeightedAvg * flSpeedRatio, -18.0f, 18.0f);
		}
	}

	// Air strafe prediction - further enhanced with Amalgam-inspired improvements for accuracy
	if (CFG::Aimbot_Projectile_AdvancedAirStrafe && !(m_PlayerDataCurrent.m_fFlags & FL_ONGROUND) && F::LagRecords->HasRecords(pPlayer))
	{
		// Increased to 10 samples for air as well
		const LagRecord_t* rec0{ F::LagRecords->GetRecord(pPlayer, 0) };
		const LagRecord_t* rec1{ F::LagRecords->GetRecord(pPlayer, 1) };
		const LagRecord_t* rec2{ F::LagRecords->GetRecord(pPlayer, 2) };
		const LagRecord_t* rec3{ F::LagRecords->GetRecord(pPlayer, 3) };
		const LagRecord_t* rec4{ F::LagRecords->GetRecord(pPlayer, 4) };
		const LagRecord_t* rec5{ F::LagRecords->GetRecord(pPlayer, 5) };
		const LagRecord_t* rec6{ F::LagRecords->GetRecord(pPlayer, 6) };
		const LagRecord_t* rec7{ F::LagRecords->GetRecord(pPlayer, 7) };
		const LagRecord_t* rec8{ F::LagRecords->GetRecord(pPlayer, 8) };
		const LagRecord_t* rec9{ F::LagRecords->GetRecord(pPlayer, 9) };

		if (rec0 && rec1 && rec2 && rec3 && rec4 && rec5 && rec6 && rec7 && rec8 && rec9)
		{
			// Same rotation calculation
			auto CalculateRotationAngle = [](const Vec3& v1, const Vec3& v2) -> float
				{
					const float len1 = v1.Length2D();
					const float len2 = v2.Length2D();

					if (len1 < 0.01f || len2 < 0.01f)
						return 0.0f;

					const float nx1 = v1.x / len1;
					const float ny1 = v1.y / len1;
					const float nx2 = v2.x / len2;
					const float ny2 = v2.y / len2;

					const float sin_theta = nx1 * ny2 - ny1 * nx2;
					const float cos_theta = nx1 * nx2 + ny1 * ny2;

					return atan2f(sin_theta, cos_theta) * (180.0f / PI);
				};

			// More deltas
			const float delta0 = CalculateRotationAngle(rec1->Velocity, rec0->Velocity);
			const float delta1 = CalculateRotationAngle(rec2->Velocity, rec1->Velocity);
			const float delta2 = CalculateRotationAngle(rec3->Velocity, rec2->Velocity);
			const float delta3 = CalculateRotationAngle(rec4->Velocity, rec3->Velocity);
			const float delta4 = CalculateRotationAngle(rec5->Velocity, rec4->Velocity);
			const float delta5 = CalculateRotationAngle(rec6->Velocity, rec5->Velocity);
			const float delta6 = CalculateRotationAngle(rec7->Velocity, rec6->Velocity);
			const float delta7 = CalculateRotationAngle(rec8->Velocity, rec7->Velocity);
			const float delta8 = CalculateRotationAngle(rec9->Velocity, rec8->Velocity);

			// Keep 45 for air (more variability)
			if (fabsf(delta0) > 45.0f || fabsf(delta1) > 45.0f || fabsf(delta2) > 45.0f ||
				fabsf(delta3) > 45.0f || fabsf(delta4) > 45.0f || fabsf(delta5) > 45.0f ||
				fabsf(delta6) > 45.0f || fabsf(delta7) > 45.0f || fabsf(delta8) > 45.0f)
			{
				return;
			}

			// Stricter direction changes for air (reduced to 2)
			int iDirectionChanges = 0;
			int iLastSign = delta0 > 0.0f ? 1 : (delta0 < 0.0f ? -1 : 0);

			const float deltas[] = { delta1, delta2, delta3, delta4, delta5, delta6, delta7, delta8 };
			for (const float delta : deltas)
			{
				const int iCurrSign = delta > 0.0f ? 1 : (delta < 0.0f ? -1 : 0);
				if (iCurrSign != 0 && iLastSign != 0 && iCurrSign != iLastSign)
				{
					iDirectionChanges++;
				}
				if (iCurrSign != 0)
				{
					iLastSign = iCurrSign;
				}
			}

			if (iDirectionChanges > 2)
			{
				return;
			}

			// Adjusted weighted average
			const float flWeightedAvg = (delta0 * 4.0f + delta1 * 3.5f + delta2 * 3.0f +
				delta3 * 2.5f + delta4 * 2.0f + delta5 * 1.5f + delta6 * 1.2f +
				delta7 * 1.0f + delta8 * 0.8f) / 19.5f;

			// Consistent threshold
			if (fabsf(flWeightedAvg) < 0.35f)
			{
				return;
			}

			// No speed scaling for air, adjusted clamp
			m_flYawTurnRate = std::clamp(flWeightedAvg, -22.0f, 22.0f);
		}
	}
}

bool CMovementSimulation::Initialize(C_TFPlayer* pPlayer)
{
	if (!pPlayer || pPlayer->deadflag())
		return false;

	//set player
	m_pPlayer = pPlayer;

	//set current command
	//we'll use this to set current player's command, without it CGameMovement::CheckInterval will try to access a nullptr
	static CUserCmd dummyCmd = {};

	I::MoveHelper->SetHost(m_pPlayer);
	m_pPlayer->SetCurrentCommand(&dummyCmd);

	//store player's data
	m_PlayerDataBackup.Store(m_pPlayer);
	m_PlayerDataCurrent.UpdateFromPlayer(m_pPlayer);

	//store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	//the hacks that make it work
	{
		if (pPlayer->m_fFlags() & FL_DUCKING)
		{
			pPlayer->m_fFlags() &= ~FL_DUCKING; //breaks origin's z if FL_DUCKING is not removed
			pPlayer->m_bDucked() = true; //(mins/maxs will be fine when ducking as long as m_bDucked is true)
			pPlayer->m_flDucktime() = 0.0f;
			pPlayer->m_flDuckJumpTime() = 0.0f;
			pPlayer->m_bDucking() = false;
			pPlayer->m_bInDuckJump() = true;
		}

		if (pPlayer != H::Entities->GetLocal())
			pPlayer->m_hGroundEntity() = nullptr; //without this nonlocal entities get snapped to the floor

		pPlayer->m_flModelScale() -= 0.03125f; //fixes issues with corners

		if (pPlayer->m_fFlags() & FL_ONGROUND)
			pPlayer->m_vecOrigin().z += 0.03125f * 3.0f; //to prevent getting stuck in the ground

		if ((pPlayer->m_fFlags() & FL_ONGROUND) || pPlayer->m_hGroundEntity().Get())
		{
			pPlayer->m_vecVelocity().z = 0.0f;
		}
	}

	//setup move data
	SetupMoveData(m_pPlayer, &m_MoveData);

	return true;
}

void CMovementSimulation::Restore()
{
	if (!m_pPlayer)
		return;

	I::MoveHelper->SetHost(nullptr);
	m_pPlayer->SetCurrentCommand(nullptr);

	m_PlayerDataBackup.Restore(m_pPlayer);

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;

	m_pPlayer = nullptr;
	m_flYawTurnRate = 0.0f;

	std::memset(&m_MoveData, 0, sizeof(CMoveData));
	std::memset(&m_PlayerDataBackup, 0, sizeof(CPlayerDataBackup));
}

void CMovementSimulation::RunTick(float flTimeToTarget)
{
	if (!m_pPlayer)
	{
		return;
	}

	m_PlayerDataCurrent.UpdateFromPlayer(m_pPlayer);

	//make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

	bool isStationary = false;

	if (F::LagRecords->HasRecords(m_pPlayer))
	{
		float avgVel = 0.0f;
		int countVel = 0;
		float maxVelChange = 0.0f;
		float minDot = 1.0f;
		int countDir = 0;

		for (int i = 0; i < 5; ++i)
		{
			auto rec = F::LagRecords->GetRecord(m_pPlayer, i);
			if (!rec) break;
			avgVel += rec->Velocity.Length2D();
			countVel++;

			if (i < 4)
			{
				auto nextRec = F::LagRecords->GetRecord(m_pPlayer, i + 1);
				if (nextRec)
				{
					float velChange = (rec->Velocity - nextRec->Velocity).Length2D();
					maxVelChange = std::max(maxVelChange, velChange);

					float len1 = rec->Velocity.Length2D();
					float len2 = nextRec->Velocity.Length2D();
					if (len1 > 3.0f && len2 > 3.0f)
					{
						Vec3 v1 = rec->Velocity;
						v1.z = 0.0f;
						v1.Normalize();
						Vec3 v2 = nextRec->Velocity;
						v2.z = 0.0f;
						v2.Normalize();
						float dot = v1.Dot(v2);
						minDot = std::min(minDot, dot);
						countDir++;
					}
				}
			}
		}
		if (countVel > 0)
		{
			avgVel /= countVel;
		}

		isStationary = (avgVel < 5.0f && maxVelChange < 3.0f && (countDir == 0 || minDot > 0.95f));

		// Additional position delta check
		auto rec0 = F::LagRecords->GetRecord(m_pPlayer, 0);
		auto rec4 = F::LagRecords->GetRecord(m_pPlayer, 4);
		if (rec0 && rec4)
		{
			float posDelta = (rec0->AbsOrigin - rec4->AbsOrigin).Length2D();
			if (posDelta > 5.0f)
			{
				isStationary = false;
			}
		}
	}
	else
	{
		// Fallback if no records
		float velLen2D = m_PlayerDataCurrent.m_vecVelocity.Length2D();
		isStationary = velLen2D < 5.0f;
	}

	if (isStationary) {
		if (m_PlayerDataCurrent.m_fFlags & FL_ONGROUND) {
			// Truly stationary on ground: no change needed
			return;
		}
		else {
			// Stationary in air: set moves to 0 and let engine handle gravity
			m_MoveData.m_flForwardMove = 0.0f;
			m_MoveData.m_flSideMove = 0.0f;
		}
	}

	// Duck speed reduction (Amalgam technique)
	float flOldMaxSpeed = m_MoveData.m_flClientMaxSpeed;
	if (m_pPlayer->m_bDucked() && (m_PlayerDataCurrent.m_fFlags & FL_ONGROUND) && (m_PlayerDataCurrent.m_nWaterLevel < 2))
	{
		m_MoveData.m_flClientMaxSpeed /= 3.0f;
	}

	m_bRunning = true;

	const int NUM_SUBSTEPS = 4;
	float original_frametime = I::GlobalVars->frametime;
	float sub_frametime = original_frametime / static_cast<float>(NUM_SUBSTEPS);
	I::GlobalVars->frametime = sub_frametime;

	// Precompute total adjustments
	float totalGroundAdjustment = 0.0f;
	float totalAirCorrection = 0.0f;
	float totalAirTurn = 0.0f;
	const bool bIsGroundedInitial = (m_PlayerDataCurrent.m_fFlags & FL_ONGROUND);
	const bool bIsAirborneInitial = !bIsGroundedInitial;

	if (m_flYawTurnRate != 0.0f)
	{
		if (CFG::Aimbot_Projectile_GroundStrafePrediction && bIsGroundedInitial)
		{
			totalGroundAdjustment = m_flYawTurnRate * Math::RemapValClamped(flTimeToTarget, 0.0f, 1.0f, 1.0f, 0.6f);
		}
		else if (CFG::Aimbot_Projectile_AdvancedAirStrafe && bIsAirborneInitial)
		{
			totalAirTurn = m_flYawTurnRate;
		}
	}

	float subGroundAdjustment = totalGroundAdjustment / static_cast<float>(NUM_SUBSTEPS);
	float airSign = (totalAirTurn > 0.0f ? 1.0f : -1.0f);
	float baseAirTurnRate = totalAirTurn;
	float airaccel = I::CVar->FindVar("sv_airaccelerate")->GetFloat();

	for (int substep = 0; substep < NUM_SUBSTEPS; ++substep)
	{
		// Update current state before each substep
		m_PlayerDataCurrent.UpdateFromPlayer(m_pPlayer);

		// Re-evaluate grounded/airborne for this substep
		const bool bIsGrounded = (m_PlayerDataCurrent.m_fFlags & FL_ONGROUND);
		const bool bIsAirborne = !bIsGrounded;

		// Apply fractional adjustment for this substep
		float flSubCorrection = 0.0f;
		if (m_flYawTurnRate != 0.0f)
		{
			if (CFG::Aimbot_Projectile_GroundStrafePrediction && bIsGrounded)
			{
				m_MoveData.m_vecViewAngles.y += subGroundAdjustment;
			}
			else if (CFG::Aimbot_Projectile_AdvancedAirStrafe && bIsAirborne)
			{
				float accel_dt = airaccel * 30.0f * sub_frametime;
				float current_speed2d = m_MoveData.m_vecVelocity.Length2D();
				if (current_speed2d < 0.01f) current_speed2d = 0.01f;
				float max_sub_turn = RAD2DEG(atanf(accel_dt / current_speed2d));

				float sub_turn = baseAirTurnRate * (sub_frametime / TICK_INTERVAL);
				sub_turn = std::clamp(sub_turn, -max_sub_turn, max_sub_turn);

				float vel_angle = Math::VelocityToAngles(m_MoveData.m_vecVelocity).y;
				flSubCorrection = 90.0f * airSign;
				m_MoveData.m_vecViewAngles.y = vel_angle + flSubCorrection + sub_turn;
			}
		}

		// Re-setup forward/side move based on current view angles
		Vec3 vForward = {}, vRight = {};
		Math::AngleVectors(m_MoveData.m_vecViewAngles, &vForward, &vRight, nullptr);
		m_MoveData.m_flForwardMove = m_MoveData.m_vecVelocity.Dot(vForward);
		m_MoveData.m_flSideMove = m_MoveData.m_vecVelocity.Dot(vRight);

		// Process the substep
		I::GameMovement->ProcessMovement(m_pPlayer, &m_MoveData);

		// Remove sub-correction after substep
		if (flSubCorrection != 0.0f)
		{
			m_MoveData.m_vecViewAngles.y -= flSubCorrection;
		}
	}

	I::GlobalVars->frametime = original_frametime;

	m_bRunning = false;

	// Restore max speed
	m_MoveData.m_flClientMaxSpeed = flOldMaxSpeed;

	m_PlayerDataCurrent.UpdateFromPlayer(m_pPlayer);
}

const Vec3& CMovementSimulation::GetSimulatedVelocity() const
{
	return m_MoveData.m_vecVelocity;
}

const Vec3& CMovementSimulation::GetOrigin() const
{
	return m_MoveData.m_vecAbsOrigin;
}


bool CMovementSimulation::IsSimulatedOnGround() const
{
	return (m_PlayerDataCurrent.m_fFlags & FL_ONGROUND) != 0;
}

const Vec3& CMovementSimulation::GetSimulatedOrigin() const
{
	return m_MoveData.m_vecAbsOrigin;
}