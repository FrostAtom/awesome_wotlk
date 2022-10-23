#include "NamePlates.h"
#include "GameClient.h"
#include <Windows.h>
#include <Detours/detours.h>
#include <vector>
#include <array>
#include <cstring>
#define NAME_PLATE_CREATED "NAME_PLATE_CREATED"
#define NAME_PLATE_UNIT_ADDED "NAME_PLATE_UNIT_ADDED"
#define NAME_PLATE_UNIT_REMOVED "NAME_PLATE_UNIT_REMOVED"

static std::array<guid_t, 80> s_nameplateGuids;
static Console::CVar* s_nameplatePlayer = NULL;

static int C_NamePlate_GetNamePlates(lua_State* L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "C_NamePlate_Cache");
    lua_wipe(L, 1);
    for (size_t i = 0, id = 1; i < std::size(s_nameplateGuids); i++) {
        if (guid_t guid = s_nameplateGuids[i]) {
            if (Object* unit = ObjectMgr::Get(s_nameplateGuids[i], ObjectFlags_Unit); unit && unit->nameplate) {
                lua_pushframe(L, unit->nameplate);
                lua_rawseti(L, 1, id++); 
            }
        }
    }

    return 1;
}

static int C_NamePlate_GetNamePlateForUnit(lua_State* L)
{
    const char* token = luaL_checkstring(L, 1);
    guid_t guid = ObjectMgr::GetGuidByUnitID(token);
    if (!guid) return 0;
    Object* unit = ObjectMgr::Get(guid, ObjectFlags_Unit);
    if (!unit || !unit->nameplate) return 0;
    lua_pushframe(L, unit->nameplate);
    return 1;
}

static int lua_IsAwesomeWotlk(lua_State* L)
{
    lua_pushnumber(L, 1.f);
    return 1;
}

static void OpenApi()
{
    lua_State* L = GetLuaState();
    luaL_Reg methods[] = {
        {"GetNamePlates", C_NamePlate_GetNamePlates},
        {"GetNamePlateForUnit", C_NamePlate_GetNamePlateForUnit},
    };
    
    lua_createtable(L, 0, std::size(methods));
    for (size_t i = 0; i < std::size(methods); i++) {
        lua_pushcfunction(L, methods[i].func);
        lua_setfield(L, -2, methods[i].name);
    }
    lua_setglobal(L, "C_NamePlate");

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_NamePlate_Cache");

    lua_pushcfunction(L, lua_IsAwesomeWotlk);
    lua_setglobal(L, "IsAwesomeWotlk");
}

static void(*Lua_OpenFrameXMLApi_orig)() = (decltype(Lua_OpenFrameXMLApi_orig))0x00530F85;
static void __declspec(naked) Lua_OpenFrameXMLApi_hk()
{
    __asm {
        pushad;
        pushfd;
        call OpenApi;
        popfd;
        popad;
        ret;
    }
}

static void HandleNamePlateHide(Object* object)
{
    guid_t guid = object->entry->guid;
    for (size_t i = 0; i < std::size(s_nameplateGuids); i++) {
        if (s_nameplateGuids[i] == guid) {
            char buf[32];
            snprintf(buf, std::size(buf), "nameplate%d", i + 1);
            FrameScript::FireEvent(NAME_PLATE_UNIT_REMOVED, "%s", buf);
            s_nameplateGuids[i] = 0;
        }
    }
}

static void HandleNamePlateShow(Object* object)
{
    guid_t guid = object->entry->guid;
    for (size_t i = 0; i < std::size(s_nameplateGuids); i++) {
        if (!s_nameplateGuids[i]) {
            s_nameplateGuids[i] = guid;
            char buf[32];
            snprintf(buf, std::size(buf), "nameplate%d", i + 1);
            FrameScript::FireEvent(NAME_PLATE_UNIT_ADDED, "%s", buf);
            break;
        }
    }
}

static void HandleNamePlateCreate(Frame* frame)
{
    lua_State* L = GetLuaState();
    lua_pushstring(L, NAME_PLATE_CREATED);
    lua_rawgeti(L, LUA_REGISTRYINDEX, frame->luaRef);
    FrameScript::FireEvent_inner(FrameScript::GetEventIdByName(NAME_PLATE_CREATED), L, 2);
    lua_pop(L, 2);
}

static void (*Unit_GetNamePlateInfo_orig)() = (decltype(Unit_GetNamePlateInfo_orig))0x0072B0F7;
static void __declspec(naked) Unit_GetNamePlateInfo_hk()
{
    // TODO
}


static bool CVarHandler_NameplateDistance(Console::CVar*, const char*, const char* value, LPVOID)
{
    double f = atof(value);
    f = f > 0.f ? f : 43.f;
    *(float*)0x00ADAA7C = (float)(f * f);
    return true;
}

static void(*CVars_Initialize_orig)() = (decltype(CVars_Initialize_orig))0x00401B60;
static void CVars_Initialize_hk()
{
    CVars_Initialize_orig();
    Console::RegisterCVar("nameplateDistance", NULL, 1, "43", CVarHandler_NameplateDistance, 0, 0, 0, 0);
    //Console::RegisterCVar("nameplatePlayer", NULL, 1, "1", NULL, 0, 0, 0, 0);
}

static void (__fastcall* NamePlate_Create_orig)(Frame* frame, void* edx, Frame* parent) = (decltype(NamePlate_Create_orig))0x0098F790;
static void __fastcall NamePlate_Create_hk(Frame* frame, void* edx, Frame* parent)
{
    NamePlate_Create_orig(frame, edx, parent);
    HandleNamePlateCreate(frame);
}

static void(__fastcall* Unit_ShowNamePlate_orig)() = (decltype(Unit_ShowNamePlate_orig))0x00725766;
static void __declspec(naked) Unit_ShowNamePlate_hk()
{
    __asm {
        pushad;
        pushfd;
        push esi;
        call HandleNamePlateShow;
        add esp, 4;
        popfd;
        popad;

        push Unit_ShowNamePlate_orig;
        ret;
    }
}

static void (*Unit_HideNamePlate_orig)() = (decltype(Unit_HideNamePlate_orig))0x0072584D;
static void __declspec(naked) Unit_HideNamePlate_hk()
{
    __asm {
        pushad;
        pushfd;
        push esi;
        call HandleNamePlateHide;
        add esp, 4;
        popfd;
        popad;

        push Unit_HideNamePlate_orig;
        ret;
    }
}

static void (*HideAllNamePlates_orig)() = (decltype(HideAllNamePlates_orig))0x00727184;
static void __declspec(naked) HideAllNamePlates_hk()
{
    __asm {
        pushad;
        pushfd;
        push esi;
        call HandleNamePlateHide;
        add esp, 4;
        popfd;
        popad;

        push HideAllNamePlates_orig;
        ret;
    }
}

static void(*Unit_dtor_orig)() = (decltype(Unit_dtor_orig))0x007350AD;
static void __declspec(naked) Unit_dtor_hk()
{
    __asm {
        pushad;
        pushfd;
        push esi;
        call HandleNamePlateHide;
        add esp, 4;
        popfd;
        popad;

        push Unit_dtor_orig;
        ret;
    }
}

static void GetGuidByKeyword_bulk(const char** stackStr, guid_t* guid)
{
    if (strncmp(*stackStr, "nameplate", 9) == 0) {
        *stackStr += 9;
        size_t id = gc_atoi(stackStr) - 1;
        if (id < std::size(s_nameplateGuids))
            *guid = s_nameplateGuids[id];
    }
}

static void(*GetGuidByKeyword_orig)() = (decltype(GetGuidByKeyword_orig))0x0060AFAA;
static void __declspec(naked) GetGuidByKeyword_hk()
{
    __asm {
        pushad;
        pushfd;
        push[ebp + 0xC];
        lea eax, [ebp + 0x8];
        push eax;
        call GetGuidByKeyword_bulk;
        add esp, 8;
        popfd;
        popad;

        push 0x0060AFDB;
        ret;
    }
}

static char** (*GetKeywordsByGuid_orig)(guid_t* guid, size_t* size) = (decltype(GetKeywordsByGuid_orig))0x0060BB70;
static char** GetKeywordsByGuid_hk(guid_t* guid, size_t* size)
{
    char** buf = GetKeywordsByGuid_orig(guid, size);

    for (size_t i = 0; i < std::size(s_nameplateGuids) && *size < 5; i++) {
        if (s_nameplateGuids[i] == *guid)
            snprintf(buf[(*size)++], 32, "nameplate%d", i + 1);
    }
    return buf;
}

static void (*FrameScript_FillEvents_orig)(const char** list, size_t count) = (decltype(FrameScript_FillEvents_orig))0x0081B5F0;
static void FrameScript_FillEvents_hk(const char** list, size_t count)
{
    std::vector<const char*> events(&list[0], &list[count]);
    events.push_back(NAME_PLATE_CREATED);
    events.push_back(NAME_PLATE_UNIT_ADDED);
    events.push_back(NAME_PLATE_UNIT_REMOVED);
    FrameScript_FillEvents_orig(events.data(), events.size());
}

void NamePlates::Initialize()
{
    DetourAttach(&(LPVOID&)Lua_OpenFrameXMLApi_orig, Lua_OpenFrameXMLApi_hk);
    DetourAttach(&(LPVOID&)CVars_Initialize_orig, CVars_Initialize_hk);
    DetourAttach(&(LPVOID&)Unit_ShowNamePlate_orig, Unit_ShowNamePlate_hk);
    DetourAttach(&(LPVOID&)Unit_HideNamePlate_orig, Unit_HideNamePlate_hk);
    DetourAttach(&(LPVOID&)HideAllNamePlates_orig, HideAllNamePlates_hk);
    DetourAttach(&(LPVOID&)Unit_dtor_orig, Unit_dtor_hk);
    DetourAttach(&(LPVOID&)GetGuidByKeyword_orig, GetGuidByKeyword_hk);
    DetourAttach(&(LPVOID&)GetKeywordsByGuid_orig, GetKeywordsByGuid_hk);
    DetourAttach(&(LPVOID&)FrameScript_FillEvents_orig, FrameScript_FillEvents_hk);
}