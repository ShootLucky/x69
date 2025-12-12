#pragma once
#include "../src/SDK/SDK.h"
#include <string>
class CRadio
{
	bool IsSpotifyRunning();
	std::string GetSpotifyCurrentTrack();
public:
	void Pause();
	void NextTrack();
	void PrevTrack();
	void VolumeUp();
	void VolumeDown();
	void Run();
};
MAKE_SINGLETON_SCOPED(CRadio, Radio, F);