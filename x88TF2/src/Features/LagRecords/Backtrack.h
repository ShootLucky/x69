#pragma once
#include "../src/SDK/SDK.h"
#include "../Misc/Misc.h"  // Include Misc.h to get IncomingSequence_t

struct LagRecord_t
{
    C_TFPlayer* Player = nullptr;
    matrix3x4_t BoneMatrix[128] = {};
    float SimulationTime = -1.0f;
    Vec3 AbsOrigin = {};
    Vec3 VecOrigin = {};
    Vec3 AbsAngles = {};
    Vec3 EyeAngles = {};
    Vec3 Velocity = {};
    Vec3 Center = {};
    int Flags = 0;
    float FeetYaw = 0.0f;
    bool bValid = false;
};

class CLagRecords
{
    std::unordered_map<C_TFPlayer*, std::deque<LagRecord_t>> m_LagRecords = {};
    bool m_bSettingUpBones = false;
    std::deque<IncomingSequence_t> m_Sequences = {};
    int m_lastincomingsequencenumber = 0;
    bool IsSimulationTimeValid(float flCurSimTime, float flCmprSimTime);
public:
    float GetFakeLatency() const;
    void AddRecord(C_TFPlayer* pPlayer);
    const LagRecord_t* GetRecord(C_TFPlayer* pPlayer, int nRecord, bool bSafe = false);
    bool HasRecords(C_TFPlayer* pPlayer, int* pTotalRecords = nullptr);
    void UpdateRecords();
    bool DiffersFromCurrent(const LagRecord_t* pRecord);
    bool IsSettingUpBones() { return m_bSettingUpBones; }
    void RecordIncomingSequence(CNetChannel* pNetChan);
    void AdjustPing(CNetChannel* pNetChan);
};
MAKE_SINGLETON_SCOPED(CLagRecords, LagRecords, F);

class CLagRecordMatrixHelper
{
    C_TFPlayer* m_pPlayer = nullptr;
    Vec3 m_vAbsOrigin = {};
    Vec3 m_vAbsAngles = {};
    matrix3x4_t m_BoneMatrix[128] = {};
    bool m_bSuccessfullyStored = false;
public:
    void Set(const LagRecord_t* pRecord);
    void Restore();
};
MAKE_SINGLETON_SCOPED(CLagRecordMatrixHelper, LagRecordMatrixHelper, F);