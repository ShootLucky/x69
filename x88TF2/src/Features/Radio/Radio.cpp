#include "radio.h"
#include "CFG.h"
#include "../Menu/Menu.h"
#include <windows.h>
#include <tlhelp32.h>
#include <mmsystem.h> // For mciSendString
#include <string>
#include <vector>
#include <thread>
#pragma comment(lib, "winmm.lib") // For mciSendStringA

bool CRadio::IsSpotifyRunning() {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }
    bool found = false;
    do {
        if (std::string(pe32.szExeFile) == "Spotify.exe") {
            found = true;
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return found;
}

std::string CRadio::GetSpotifyCurrentTrack() {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return "";
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return "";
    }
    DWORD spotifyPid = 0;
    do {
        if (std::string(pe32.szExeFile) == "Spotify.exe") {
            spotifyPid = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    if (spotifyPid == 0) return "";
    // Find the Spotify window
    struct WindowData {
        DWORD pid;
        HWND hwnd;
    };
    WindowData data = { spotifyPid, NULL };
    struct EnumWindowsHelper {
        static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lParam) {
            WindowData* pData = reinterpret_cast<WindowData*>(lParam);
            DWORD winPid = 0;
            GetWindowThreadProcessId(hwnd, &winPid);
            if (winPid == pData->pid && IsWindowVisible(hwnd)) {
                pData->hwnd = hwnd;
                return FALSE;
            }
            return TRUE;
        }
    };
    EnumWindows(EnumWindowsHelper::EnumProc, reinterpret_cast<LPARAM>(&data));
    if (!data.hwnd) return "";
    char buf[1024] = { 0 };
    GetWindowTextA(data.hwnd, buf, sizeof(buf));
    std::string title(buf);
    // If title is just "Spotify Premium" or similar, no song is playing
    if (title.find(" - ") == std::string::npos) {
        return "No song playing";
    }
    return title;
}

void CRadio::Pause() {
    if (!IsSpotifyRunning()) return;
    keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
    keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
}

void CRadio::NextTrack() {
    if (!IsSpotifyRunning()) return;
    keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
    keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_KEYUP, 0);
}

void CRadio::PrevTrack() {
    if (!IsSpotifyRunning()) return;
    keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
    keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_KEYUP, 0);
}

void CRadio::VolumeUp() {
    keybd_event(VK_VOLUME_UP, 0, 0, 0);
    keybd_event(VK_VOLUME_UP, 0, KEYEVENTF_KEYUP, 0);
}

void CRadio::VolumeDown() {
    keybd_event(VK_VOLUME_DOWN, 0, 0, 0);
    keybd_event(VK_VOLUME_DOWN, 0, KEYEVENTF_KEYUP, 0);
}

void CRadio::Run() {
    if (!CFG::Radio) return;

    if (CFG::Radio_Pause) {
        Pause();
        CFG::Radio_Pause = false;
    }
    if (CFG::Radio_Next) {
        NextTrack();
        CFG::Radio_Next = false;
    }
    if (CFG::Radio_Prev) {
        PrevTrack();
        CFG::Radio_Prev = false;
    }
    if (CFG::Radio_VolUp) {
        VolumeUp();
        CFG::Radio_VolUp = false;
    }
    if (CFG::Radio_VolDown) {
        VolumeDown();
        CFG::Radio_VolDown = false;
    }

    if (CFG::Radio_LocalMusic) {
        static std::string current_song = "";
        static DWORD last_check = 0;
        DWORD current_time = timeGetTime();
        if (current_time - last_check > 1000) { // Check every 1 second
            last_check = current_time;
            if (IsSpotifyRunning()) {
                current_song = GetSpotifyCurrentTrack();
            }
            else {
                current_song = "";
            }
        }

        if (!current_song.empty()) {
            int screen_w, screen_h;
            I::EngineClient->GetScreenSize(screen_w, screen_h);
            const CFont& font = H::Fonts->Get(EFonts::Menu);
            int text_w = font.GetStringWidth(current_song.c_str());
            H::Draw->String(font, screen_w / 2 - text_w / 2, screen_h - 30, Color_t(255, 255, 255, 255), POS_DEFAULT, current_song.c_str());
        }
    }
}