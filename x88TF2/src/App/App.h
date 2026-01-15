#pragma once
#include <Windows.h> // For GetTickCount, etc.
#include <string>
#include "../src/SDK/SDK.h" // Assuming this includes Color_t, etc.
// Forward declarations if needed
class CApp {
public:
	void Start();
	void Loop();
	void Shutdown();
	void Draw();
private:
	const char* g_SubText = nullptr;
	bool g_SubTextChosen = false;
	bool bUnload = false;
	bool bBlackScreen = false;
	DWORD ulBlackStart = 0;
	bool fadingIn = false;
	bool fadingOut = false;
	float blackAlpha = 0.0f;
	std::string m_PlayerName;
};
MAKE_SINGLETON(CApp, App);


inline bool alive = true;