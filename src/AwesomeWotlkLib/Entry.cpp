#include "NamePlates.h"
#include "Hooks.h"
#include <Windows.h>
#include <Detours/detours.h>

static int lua_openawesomewotlk(lua_State* L)
{
    lua_pushnumber(L, 1.f);
    lua_setglobal(L, "AwesomeWotlk");
    return 0;
}

static void OnAttach()
{
    // Invalid function pointer hack
    *(DWORD*)0x00D415B8 = 1;
    *(DWORD*)0x00D415BC = 0x7FFFFFFF;

    // Initialize modules
    DetourTransactionBegin();
    Hooks::initialize();
    NamePlates::initialize();
    DetourTransactionCommit();

    // Register base
    Hooks::FrameXML::registerLuaLib(lua_openawesomewotlk);
}

int __stdcall DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        OnAttach();
    return 1;
}