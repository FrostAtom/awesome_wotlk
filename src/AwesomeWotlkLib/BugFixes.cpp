#include "BugFixes.h"
#include <Windows.h>
#include "Utils.h"
#include <Detours/detours.h>
#include <string>


inline void* alloc(size_t size) { return ((decltype(&alloc))0x00415074)(size); }

static const char* (*Clipboard_GetString_orig)(HWND hwnd) = (decltype(Clipboard_GetString_orig))0x008726F0;
static const char* Clipboard_GetString_hk(HWND hwnd)
{
    std::string str = GetFromClipboardU8(hwnd);
    char* buf = (char*)alloc(str.size());
    memcpy(buf, str.data(), str.size());
    return buf;
}

static BOOL(*Clipboard_SetString_orig)(const char* buf, HWND hwnd) = (decltype(Clipboard_SetString_orig))0x008727E0;
static BOOL Clipboard_SetString_hk(const char* buf, HWND hwnd) { return CopyToClipboardU8(buf, hwnd); }

void BugFixes::initialize()
{
    DetourAttach(&(LPVOID&)Clipboard_GetString_orig, Clipboard_GetString_hk);
    DetourAttach(&(LPVOID&)Clipboard_SetString_orig, Clipboard_SetString_hk);
}