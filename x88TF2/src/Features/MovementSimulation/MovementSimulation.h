// MovementSimulation.h
#pragma once

#include "../src/SDK/SDK.h"

class CMovementSimulation
{
    class CPlayerDataBackup
    {
    public:
        Vec3 m_vecOrigin{};
        Vec3 m_vecVelocity{};
        Vec3 m_vecBaseVelocity{};
        Vec3 m_vecViewOffset{};
        C_BaseEntity* m_hGroundEntity{};
        int m_fFlags{};
        float m_flDucktime{};
        float m_flDuckJumpTime{};
        bool m_bDucked{};
        bool m_bDucking{};
        bool m_bInDuckJump{};
        float m_flModelScale{};
        int m_nButtons{};
        float m_flLastMovementStunChange{};
        float m_flStunLerpTarget{};
        bool m_bStunNeedsFadeOut{};
        float m_flPrevTauntYaw{};
        float m_flTauntYaw{};
        float m_flCurrentTauntMoveSpeed{};
        int m_iKartState{};
        float m_flVehicleReverseTime{};
        float m_flHypeMeter{};
        float m_flMaxspeed{};
        int m_nAirDucked{};
        bool m_bJumping{};
        int m_iAirDash{};
        float m_flWaterJumpTime{};
        float m_flSwimSoundTime{};
        int m_surfaceProps{};
        void* m_pSurfaceData{};
        float m_surfaceFriction{};
        char m_chTextureType{};
        Vec3 m_vecPunchAngle{};
        Vec3 m_vecPunchAngleVel{};
        float m_flJumpTime{};
        unsigned char m_MoveType{};
        unsigned char m_MoveCollide{};
        Vec3 m_vecLadderNormal{};
        float m_flGravity{};
        unsigned char m_nWaterLevel{};
        unsigned char m_nWaterType{};
        float m_flFallVelocity{};
        int m_nPlayerCond{};
        int m_nPlayerCondEx{};
        int m_nPlayerCondEx2{};
        int m_nPlayerCondEx3{};
        int m_nPlayerCondEx4{};
        int _condition_bits{};

        void Store(C_TFPlayer* pPlayer);
        void Restore(C_TFPlayer* pPlayer);
    };

    class CPlayerDataCurrent
    {
    public:
        Vec3 m_vecOrigin{};
        Vec3 m_vecVelocity{};
        Vec3 m_vecBaseVelocity{};
        Vec3 m_vecViewOffset{};
        C_BaseEntity* m_hGroundEntity{};
        int m_fFlags{};
        float m_flDucktime{};
        float m_flDuckJumpTime{};
        bool m_bDucked{};
        bool m_bDucking{};
        bool m_bInDuckJump{};
        float m_flModelScale{};
        int m_nButtons{};
        float m_flLastMovementStunChange{};
        float m_flStunLerpTarget{};
        bool m_bStunNeedsFadeOut{};
        float m_flPrevTauntYaw{};
        float m_flTauntYaw{};
        float m_flCurrentTauntMoveSpeed{};
        int m_iKartState{};
        float m_flVehicleReverseTime{};
        float m_flHypeMeter{};
        float m_flMaxspeed{};
        int m_nAirDucked{};
        bool m_bJumping{};
        int m_iAirDash{};
        float m_flWaterJumpTime{};
        float m_flSwimSoundTime{};
        int m_surfaceProps{};
        void* m_pSurfaceData{};
        float m_surfaceFriction{};
        char m_chTextureType{};
        Vec3 m_vecPunchAngle{};
        Vec3 m_vecPunchAngleVel{};
        float m_flJumpTime{};
        unsigned char m_MoveType{};
        unsigned char m_MoveCollide{};
        Vec3 m_vecLadderNormal{};
        float m_flGravity{};
        unsigned char m_nWaterLevel{};
        unsigned char m_nWaterType{};
        float m_flFallVelocity{};
        int m_nPlayerCond{};
        int m_nPlayerCondEx{};
        int m_nPlayerCondEx2{};
        int m_nPlayerCondEx3{};
        int m_nPlayerCondEx4{};
        int _condition_bits{};

        void UpdateFromPlayer(C_TFPlayer* pPlayer);
    };

private:
    CPlayerDataBackup m_PlayerDataBackup{};
    CPlayerDataCurrent m_PlayerDataCurrent{};
    C_TFPlayer* m_pPlayer{};
    CMoveData m_MoveData{};
    bool m_bRunning{};
    float m_flYawTurnRate{};

    bool m_bOldInPrediction{};
    bool m_bOldFirstTimePredicted{};
    float m_flOldFrametime{};

    void SetupMoveData(C_TFPlayer* pPlayer, CMoveData* pMoveData);

public:
    bool Initialize(C_TFPlayer* pPlayer);
    void Restore();
    void RunTick(float flTimeToTarget = 0.0f);

    // ===== Amalgam-style read-only interface =====
    const Vec3& GetSimulatedVelocity() const;
    bool IsSimulatedOnGround() const;
    const Vec3& GetSimulatedOrigin() const;

    // ===== Legacy compatibility (do NOT remove) =====
    const Vec3& GetOrigin() const;

    bool IsRunning() const { return m_bRunning; }
};

MAKE_SINGLETON_SCOPED(CMovementSimulation, MovementSimulation, F);