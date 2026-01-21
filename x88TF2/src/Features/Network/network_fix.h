#pragma once

#include "../src/SDK/SDK.h"

class CReadPacketState
{
	float m_flFrameTimeClientState = 0.0f;
	float m_flFrameTime = 0.0f;
	float m_flCurTime = 0.0f;
	int m_nTickCount = 0;

public:
	void Store();
	void Restore();
};

class CNetworkFix
{
	CReadPacketState m_State = {};

public:
	void FixInputDelay(bool bFinalTick);
	bool ShouldReadPackets();
};
inline CNetworkFix* g_network_fix = new CNetworkFix();
