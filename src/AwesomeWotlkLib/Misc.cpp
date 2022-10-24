#include "Misc.h"
#include "Hooks.h"
#include <Windows.h>
#include <Detours/detours.h>
#define M_PI           3.14159265358979323846
#undef min
#undef max

Console::CVar* s_cvar_cameraFov = NULL;


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
    DetourAttach(&(LPVOID&)Camera_Initialize_orig, Camera_Initialize_hk);
}