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
    m_flJumpTime = pPlayer->m_flJumpTime(); // Added missing assignment
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

void CMovementSimulation::SetupMoveData(C_TFPlayer* pPlayer)
{
    if (!pPlayer)
        return;

    m_vecViewAngles = { 0.0f, Math::VelocityToAngles(pPlayer->m_vecVelocity()).y, 0.0f };

    float flMaxSpeed = pPlayer->TeamFortress_CalculateMaxSpeed();

    if (m_PlayerDataBackup.m_fFlags & FL_DUCKING)
        flMaxSpeed *= 0.3333f;

    if (CFG::Aimbot_Projectile_PredictionMethod == 0)
    {
        m_flForwardMove = 450.0f;
        m_flSideMove = 0.0f;
    }
    else
    {
        Vec3 vForward = {}, vRight = {};
        Math::AngleVectors(m_vecViewAngles, &vForward, &vRight, nullptr);

        m_flForwardMove = (pPlayer->m_vecVelocity().y - vRight.y / vRight.x * pPlayer->m_vecVelocity().x) / (vForward.y - vRight.y / vRight.x * vForward.x);
        m_flSideMove = (pPlayer->m_vecVelocity().x - vForward.x * m_flForwardMove) / vRight.x;
    }

    const float flSpeed = pPlayer->m_vecVelocity().Length2D();

    if (flSpeed <= flMaxSpeed * 0.1f)
        m_flForwardMove = m_flSideMove = 0.0f;

    m_flYawTurnRate = 0.0f;

    // Ground strafe prediction - enhanced from Amalgam
    if (CFG::Aimbot_Projectile_GroundStrafePrediction && (m_PlayerDataBackup.m_fFlags & FL_ONGROUND) && F::LagRecords->HasRecords(pPlayer))
    {
        // Don't predict if moving too slowly (likely standing still or minimal movement)
        const float flMinSpeed = flMaxSpeed * 0.4f;
        if (pPlayer->m_vecVelocity().Length2D() < flMinSpeed)
        {
            return;
        }

        // Use 8 samples for more robust detection (Amalgam uses configurable samples)
        const auto pRecord0 = F::LagRecords->GetRecord(pPlayer, 0);
        const auto pRecord1 = F::LagRecords->GetRecord(pPlayer, 1);
        const auto pRecord2 = F::LagRecords->GetRecord(pPlayer, 2);
        const auto pRecord3 = F::LagRecords->GetRecord(pPlayer, 3);
        const auto pRecord4 = F::LagRecords->GetRecord(pPlayer, 4);
        const auto pRecord5 = F::LagRecords->GetRecord(pPlayer, 5);
        const auto pRecord6 = F::LagRecords->GetRecord(pPlayer, 6);
        const auto pRecord7 = F::LagRecords->GetRecord(pPlayer, 7);

        if (pRecord0 && pRecord1 && pRecord2 && pRecord3 && pRecord4 && pRecord5 && pRecord6 && pRecord7)
        {
            // Use vector-based rotation calculation (inspired by jvnkbinv1)
            // This avoids angle normalization issues and is more mathematically robust
            auto CalculateRotationAngle = [](const Vec3& v1, const Vec3& v2) -> float
                {
                    const float len1 = v1.Length2D();
                    const float len2 = v2.Length2D();

                    // Avoid division by zero
                    if (len1 < 0.01f || len2 < 0.01f)
                        return 0.0f;

                    // Normalize to unit vectors
                    const float nx1 = v1.x / len1;
                    const float ny1 = v1.y / len1;
                    const float nx2 = v2.x / len2;
                    const float ny2 = v2.y / len2;

                    // Calculate rotation using cross product (sin) and dot product (cos)
                    const float sin_theta = nx1 * ny2 - ny1 * nx2; // 2D cross product (z-component)
                    const float cos_theta = nx1 * nx2 + ny1 * ny2; // dot product

                    // atan2 gives us the signed angle in radians, convert to degrees
                    return atan2f(sin_theta, cos_theta) * (180.0f / 3.14159265f);
                };

            // Calculate rotation deltas between consecutive velocity samples
            const float flDelta0 = CalculateRotationAngle(pRecord1->Velocity, pRecord0->Velocity);
            const float flDelta1 = CalculateRotationAngle(pRecord2->Velocity, pRecord1->Velocity);
            const float flDelta2 = CalculateRotationAngle(pRecord3->Velocity, pRecord2->Velocity);
            const float flDelta3 = CalculateRotationAngle(pRecord4->Velocity, pRecord3->Velocity);
            const float flDelta4 = CalculateRotationAngle(pRecord5->Velocity, pRecord4->Velocity);
            const float flDelta5 = CalculateRotationAngle(pRecord6->Velocity, pRecord5->Velocity);
            const float flDelta6 = CalculateRotationAngle(pRecord7->Velocity, pRecord6->Velocity);

            // Reject if any delta is too large (likely direction change or teleport)
            if (fabsf(flDelta0) > 45.0f || fabsf(flDelta1) > 45.0f || fabsf(flDelta2) > 45.0f ||
                fabsf(flDelta3) > 45.0f || fabsf(flDelta4) > 45.0f || fabsf(flDelta5) > 45.0f || fabsf(flDelta6) > 45.0f)
            {
                return;
            }

            // Check for consistent strafe direction (Amalgam-style validation)
            // Allow up to 2 direction changes for more lenient detection
            int iDirectionChanges = 0;
            int iLastSign = flDelta0 > 0.0f ? 1 : (flDelta0 < 0.0f ? -1 : 0);

            const float deltas[] = { flDelta1, flDelta2, flDelta3, flDelta4, flDelta5, flDelta6 };
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

            // Reject if too many direction changes (not consistent strafing)
            if (iDirectionChanges > 2)
            {
                return;
            }

            // Calculate weighted average (recent samples weighted more heavily)
            const float flWeightedAvg = (flDelta0 * 3.0f + flDelta1 * 2.5f + flDelta2 * 2.0f +
                flDelta3 * 1.5f + flDelta4 * 1.2f + flDelta5 * 1.0f + flDelta6 * 0.8f) / 12.0f;

            // Minimum threshold to avoid noise (Amalgam uses 0.36 degrees)
            if (fabsf(flWeightedAvg) < 0.4f)
            {
                return;
            }

            // Velocity-based scaling: slower movement = less reliable prediction
            const float flSpeedRatio = std::clamp(pPlayer->m_vecVelocity().Length2D() / flMaxSpeed, 0.5f, 1.0f);

            // Store the turn rate for use in RunTick (scaled by speed)
            m_flYawTurnRate = std::clamp(flWeightedAvg * flSpeedRatio, -20.0f, 20.0f);
        }
    }

    // Air strafe prediction - enhanced from Amalgam
    if (CFG::Aimbot_Projectile_AdvancedAirStrafe && !(m_PlayerDataBackup.m_fFlags & FL_ONGROUND) && F::LagRecords->HasRecords(pPlayer))
    {
        // Use 8 samples for air strafing (more samples = better detection)
        const LagRecord_t* rec0{ F::LagRecords->GetRecord(pPlayer, 0) };
        const LagRecord_t* rec1{ F::LagRecords->GetRecord(pPlayer, 1) };
        const LagRecord_t* rec2{ F::LagRecords->GetRecord(pPlayer, 2) };
        const LagRecord_t* rec3{ F::LagRecords->GetRecord(pPlayer, 3) };
        const LagRecord_t* rec4{ F::LagRecords->GetRecord(pPlayer, 4) };
        const LagRecord_t* rec5{ F::LagRecords->GetRecord(pPlayer, 5) };
        const LagRecord_t* rec6{ F::LagRecords->GetRecord(pPlayer, 6) };
        const LagRecord_t* rec7{ F::LagRecords->GetRecord(pPlayer, 7) };

        if (rec0 && rec1 && rec2 && rec3 && rec4 && rec5 && rec6 && rec7)
        {
            // Use vector-based rotation calculation (inspired by jvnkbinv1)
            // This avoids angle normalization issues and is more mathematically robust
            auto CalculateRotationAngle = [](const Vec3& v1, const Vec3& v2) -> float
                {
                    const float len1 = v1.Length2D();
                    const float len2 = v2.Length2D();

                    // Avoid division by zero
                    if (len1 < 0.01f || len2 < 0.01f)
                        return 0.0f;

                    // Normalize to unit vectors
                    const float nx1 = v1.x / len1;
                    const float ny1 = v1.y / len1;
                    const float nx2 = v2.x / len2;
                    const float ny2 = v2.y / len2;

                    // Calculate rotation using cross product (sin) and dot product (cos)
                    const float sin_theta = nx1 * ny2 - ny1 * nx2; // 2D cross product (z-component)
                    const float cos_theta = nx1 * nx2 + ny1 * ny2; // dot product

                    // atan2 gives us the signed angle in radians, convert to degrees
                    return atan2f(sin_theta, cos_theta) * (180.0f / 3.14159265f);
                };

            // Calculate rotation deltas between consecutive velocity samples
            const float delta0 = CalculateRotationAngle(rec1->Velocity, rec0->Velocity);
            const float delta1 = CalculateRotationAngle(rec2->Velocity, rec1->Velocity);
            const float delta2 = CalculateRotationAngle(rec3->Velocity, rec2->Velocity);
            const float delta3 = CalculateRotationAngle(rec4->Velocity, rec3->Velocity);
            const float delta4 = CalculateRotationAngle(rec5->Velocity, rec4->Velocity);
            const float delta5 = CalculateRotationAngle(rec6->Velocity, rec5->Velocity);
            const float delta6 = CalculateRotationAngle(rec7->Velocity, rec6->Velocity);

            // Reject if any delta is too large (likely direction change)
            if (fabsf(delta0) > 45.0f || fabsf(delta1) > 45.0f || fabsf(delta2) > 45.0f ||
                fabsf(delta3) > 45.0f || fabsf(delta4) > 45.0f || fabsf(delta5) > 45.0f || fabsf(delta6) > 45.0f)
            {
                return;
            }

            // Check for consistent air strafe direction (allow up to 3 changes for air)
            int iDirectionChanges = 0;
            int iLastSign = delta0 > 0.0f ? 1 : (delta0 < 0.0f ? -1 : 0);

            const float deltas[] = { delta1, delta2, delta3, delta4, delta5, delta6 };
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

            // Air strafing can have more direction changes than ground
            if (iDirectionChanges > 3)
            {
                return;
            }

            // Calculate weighted average (air strafing benefits from recent data)
            const float flWeightedAvg = (delta0 * 3.0f + delta1 * 2.5f + delta2 * 2.0f +
                delta3 * 1.5f + delta4 * 1.2f + delta5 * 1.0f + delta6 * 0.8f) / 12.0f;

            // Minimum threshold for air strafing (lower than ground, Amalgam uses 0.36)
            if (fabsf(flWeightedAvg) < 0.35f)
            {
                return;
            }

            // Air strafing: no speed scaling needed (air control is consistent)
            // Store the turn rate for use in RunTick
            m_flYawTurnRate = std::clamp(flWeightedAvg, -25.0f, 25.0f);
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

        //for some reason if xy vel is zero it doesn't predict
        if (fabsf(pPlayer->m_vecVelocity().x) < 0.01f)
            pPlayer->m_vecVelocity().x = 0.015f;

        if (fabsf(pPlayer->m_vecVelocity().y) < 0.01f)
            pPlayer->m_vecVelocity().y = 0.015f;

        if ((pPlayer->m_fFlags() & FL_ONGROUND) || pPlayer->m_hGroundEntity().Get())
        {
            pPlayer->m_vecVelocity().z = 0.0f;
        }
    }

    //setup move data
    SetupMoveData(m_pPlayer);

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
    m_vecViewAngles = {};
    m_flForwardMove = 0.0f;
    m_flSideMove = 0.0f;

    std::memset(&m_MoveData, 0, sizeof(CMoveData));
    std::memset(&m_PlayerDataBackup, 0, sizeof(CPlayerDataBackup));
}

void CMovementSimulation::RunTick(float flTimeToTarget)
{
    if (!m_pPlayer)
        return;

    //make sure frametime and prediction vars are right
    I::Prediction->m_bInPrediction = true;
    I::Prediction->m_bFirstTimePredicted = false;
    I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

    // Early exit for stationary grounded players (performance optimization)
    if (m_pPlayer->m_vecVelocity().Length() < 15.0f && (m_pPlayer->m_fFlags() & FL_ONGROUND))
    {
        return;
    }

    // Apply strafe prediction with Amalgam-style corrections
    float flCorrection = 0.0f;
    const bool bIsGrounded = (m_PlayerDataBackup.m_fFlags & FL_ONGROUND) && (m_pPlayer->m_fFlags() & FL_ONGROUND);
    const bool bIsAirborne = !(m_PlayerDataBackup.m_fFlags & FL_ONGROUND) && !(m_pPlayer->m_fFlags() & FL_ONGROUND);

    if (m_flYawTurnRate != 0.0f)
    {
        if (CFG::Aimbot_Projectile_GroundStrafePrediction && bIsGrounded)
        {
            // Ground strafe: Apply with time-based scaling
            m_vecViewAngles.y += m_flYawTurnRate * Math::RemapValClamped(flTimeToTarget, 0.0f, 1.0f, 1.0f, 0.5f);
        }
        else if (CFG::Aimbot_Projectile_AdvancedAirStrafe && bIsAirborne)
        {
            // Air strafe: Apply 90-degree correction (Amalgam technique)
            // This accounts for the perpendicular relationship between view angle and strafe direction
            flCorrection = 90.0f * (m_flYawTurnRate > 0.0f ? 1.0f : -1.0f);
            m_vecViewAngles.y += m_flYawTurnRate + flCorrection;
        }
    }

    static CUserCmd dummyCmd = {};
    dummyCmd.forwardmove = m_flForwardMove;
    dummyCmd.sidemove = m_flSideMove;
    dummyCmd.buttons = m_PlayerDataBackup.m_nButtons;
    dummyCmd.viewangles = m_vecViewAngles;

    I::Prediction->SetupMove(m_pPlayer, &dummyCmd, I::MoveHelper, &m_MoveData);

    // Duck speed reduction (Amalgam technique)
    float flOldMaxSpeed = m_MoveData.m_flClientMaxSpeed;
    if (m_pPlayer->m_bDucked() && (m_pPlayer->m_fFlags() & FL_ONGROUND) && (m_pPlayer->m_nWaterLevel() < 2))
    {
        m_MoveData.m_flClientMaxSpeed /= 3.0f;
    }

    m_bRunning = true;

    I::GameMovement->ProcessMovement(m_pPlayer, &m_MoveData);

    I::Prediction->FinishMove(m_pPlayer, &dummyCmd, &m_MoveData);

    m_bRunning = false;

    // Restore max speed
    m_MoveData.m_flClientMaxSpeed = flOldMaxSpeed;

    // Remove air strafe correction after simulation
    if (flCorrection != 0.0f)
    {
        m_vecViewAngles.y -= flCorrection;
    }
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
    return (m_pPlayer->m_fFlags() & FL_ONGROUND) != 0;
}

const Vec3& CMovementSimulation::GetSimulatedOrigin() const
{
    return m_MoveData.m_vecAbsOrigin;
}