#include "../../SDK/SDK.h"
#include <vector>
class CMisc
{
public:
	void Bunnyhop(CUserCmd* pCmd);
	void AutoRocketJump(CUserCmd* pCmd);
	void AutoStrafe(CUserCmd* pCmd);
};
MAKE_SINGLETON_SCOPED(CMisc, Misc, F);