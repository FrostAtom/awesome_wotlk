#include "Misc.h"
#include "GameClient.h"
#include "Hooks.h"
#include "Utils.h"
#include <Windows.h>
#include <Detours/detours.h>
#define M_PI           3.14159265358979323846
#undef min
#undef max

static Console::CVar* s_cvar_cameraFov = NULL;


static int lua_FlashWindow(lua_State* L)
{
    HWND hwnd = GetGameWindow();
    if (hwnd) FlashWindow(hwnd, FALSE);
    return 0;
}

static int lua_IsWindowFocused(lua_State* L)
{
    HWND hwnd = GetGameWindow();
    if (!hwnd || GetForegroundWindow() != hwnd)
        return 0;
    lua_pushnumber(L, 1.f);
    return 1;
}

static int lua_FocusWindow(lua_State* L)
{
    HWND hwnd = GetGameWindow();
    if (hwnd) SetForegroundWindow(hwnd);
    return 0;
}

static int lua_CopyToClipboard(lua_State* L)
{
    const char* str = luaL_checkstring(L, 1);
    if (str && str[0]) CopyToClipboardU8(str, NULL);
    return 0;
}

static int lua_openmisclib(lua_State* L)
{
    luaL_Reg funcs[] = {
        { "FlashWindow", lua_FlashWindow },
        { "IsWindowFocused", lua_IsWindowFocused },
        { "FocusWindow", lua_FocusWindow },
        { "CopyToClipboard", lua_CopyToClipboard },
    };

    for (const auto& [name, func] : funcs) {
        lua_pushcfunction(L, func);
        lua_setglobal(L, name);
    }

    return 0;
}

static double parseFov(const char* v) { return  M_PI / 200.f * double(std::max(std::min(gc_atoi(&v), 200), 1)); }

static int CVarHandler_cameraFov(Console::CVar* cvar, const char* prevV, const char* newV, void* udata)
{
    if (Camera* camera = GetActiveCamera()) camera->fovInRadians = parseFov(newV);
    return 1;
}

static void(__fastcall* Camera_Initialize_orig)(Camera* self, void* edx, float a2, float a3, float fov) = (decltype(Camera_Initialize_orig))0x00607C20;
static void __fastcall Camera_Initialize_hk(Camera* self, void* edx, float a2, float a3, float fov)
{
    fov = parseFov(s_cvar_cameraFov->vStr);
    Camera_Initialize_orig(self, edx, a2, a3, fov);
}

void Misc::initialize()
{
    Hooks::FrameXML::registerCVar(&s_cvar_cameraFov, "cameraFov", NULL, (Console::CVarFlags)1, "100", CVarHandler_cameraFov);
    Hooks::FrameXML::registerLuaLib(lua_openmisclib);

    DetourAttach(&(LPVOID&)Camera_Initialize_orig, Camera_Initialize_hk);
}