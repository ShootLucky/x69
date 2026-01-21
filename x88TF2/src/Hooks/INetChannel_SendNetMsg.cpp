#include "../src/SDK/SDK.h"
#include "../Features/LagRecords/Backtrack.h"

MAKE_SIGNATURE(CNetChannel_SendDatagram, "engine.dll", "40 55 57 41 56 48 8D AC 24", 0x0);

MAKE_HOOK(CNetChannel_SendDatagram, Signatures::CNetChannel_SendDatagram.Get(), int, __fastcall,
	CNetChannel* pNetChan, bf_write* datagram)
{
	// Only apply fake latency when datagram is NULL (actual packet send)
	if (!datagram)
	{
		// Apply fake latency before sending
		F::LagRecords->AdjustPing(pNetChan);

		const int iReturn = CALL_ORIGINAL(pNetChan, datagram);

		// Restore real ping after sending
		F::LagRecords->RestorePing(pNetChan);

		return iReturn;
	}

	return CALL_ORIGINAL(pNetChan, datagram);
}