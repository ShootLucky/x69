#pragma once

#include "../../SDK/SDK.h"
#include <vector>

class CMisc
{
public:
    void Bunnyhop(CUserCmd* pCmd);
};

MAKE_SINGLETON_SCOPED(CMisc, Misc, F);
