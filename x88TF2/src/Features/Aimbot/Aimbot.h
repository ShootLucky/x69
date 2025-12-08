#pragma once

#include "../src/SDK/SDK.h"

class CAimbot
{
	void RunMain(CUserCmd* pCmd);

public:
	void Run(CUserCmd* pCmd);
};

MAKE_SINGLETON_SCOPED(CAimbot, Aimbot, F);