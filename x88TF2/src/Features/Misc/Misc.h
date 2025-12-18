#pragma once
#include "../../SDK/SDK.h"
#include <vector>
#include <deque>
struct IncomingSequence_t
{
	int inreliablestate = 0;
	int sequencenr = 0;
	float curtime = 0.0f;
};
class CMisc
{
	std::deque<IncomingSequence_t> m_Sequences = {};
	int m_lastincomingsequencenumber = 0;
public:
	void Bunnyhop(CUserCmd* pCmd);
	void AutoRocketJump(CUserCmd* pCmd);
	void AutoStrafe(CUserCmd* pCmd);
	void AntiAFK(CUserCmd* pCmd);
	float GetFakeLatency() const;
	void RecordIncomingSequence(CNetChannel* pNetChan);
	void AdjustPing(CNetChannel* pNetChan);
	void Thirdperson(CViewSetup* pSetup);
	void ViewModelOffsets(); // sem pSetup
};
MAKE_SINGLETON_SCOPED(CMisc, Misc, F);