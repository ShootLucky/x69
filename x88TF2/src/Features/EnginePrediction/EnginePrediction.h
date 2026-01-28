#pragma once

#include "../src/SDK/SDK.h"

class CEnginePrediction
{
	CMoveData m_MoveData = {};
	float m_fOldCurrentTime = 0.0f;
	float m_fOldFrameTime = 0.0f;
	int m_nOldTickCount = 0;

	int GetTickbase(CUserCmd* pCmd, C_TFPlayer* pLocal);

	// Event suppression for prediction (credits: seo64)
	void SuppressEvents(C_BaseEntity* pEntity);

public:
	int flags{};
	bool m_bInPrediction = false;

	// Store old values for restoration
	int m_nOldTickBase = 0;
	bool m_bOldIsFirstPrediction = false;
	bool m_bOldInPrediction = false;
	Vec3 m_vecOldVelocity = {};

	void Start(CUserCmd* pCmd);
	void End();

	// Helper to check if we're currently in prediction
	inline bool InPrediction() const { return m_bInPrediction; }
};

MAKE_SINGLETON_SCOPED(CEnginePrediction, EnginePrediction, F)