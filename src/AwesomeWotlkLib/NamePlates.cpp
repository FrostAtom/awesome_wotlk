#include "NamePlates.h"
#include "GameClient.h"
#include "Hooks.h"
#include <Windows.h>
#include <Detours/detours.h>
#include <vector>
#include <array>
#include <cstring>
#define NAME_PLATE_CREATED "NAME_PLATE_CREATED"
#define NAME_PLATE_UNIT_ADDED "NAME_PLATE_UNIT_ADDED"
#define NAME_PLATE_UNIT_REMOVED "NAME_PLATE_UNIT_REMOVED"

static std::array<guid_t, 80> s_nameplateGuids;
static Console::CVar* s_cvar_nameplateDistance;

static guid_t getTokenGuid(int id)
{
    if (id >= std::size(s_nameplateGuids))
        return 0;
    return s_nameplateGuids[id];
}

static int getTokenId(guid_t guid)
{
    for (size_t i = 0; i < std::size(s_nameplateGuids); i++) {
        if (s_nameplateGuids[i] == guid)
            return i;
    }
    return -1;
}


static int CVarHandler_NameplateDistance(Console::CVar*, const char*, const char* value, LPVOID)
{
    double f = atof(value);
    f = f > 0.f ? f : 43.f;
    *(float*)0x00ADAA7C = (float)(f * f);
    return 1;
}

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

static int lua_openlibnameplates(lua_State* L)
{
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
    lua_setfield(L, LUA_REGISTRYINDEX, "C_NamePlate_Cache"); // TODO: implement table pools
    return 0;
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

void NamePlates::initialize()
{
    Hooks::FrameXML::registerLuaLib(lua_openlibnameplates);
    Hooks::FrameXML::registerEvent(NAME_PLATE_CREATED);
    Hooks::FrameXML::registerEvent(NAME_PLATE_UNIT_ADDED);
    Hooks::FrameXML::registerEvent(NAME_PLATE_UNIT_REMOVED);
    Hooks::FrameXML::registerCVar(&s_cvar_nameplateDistance, "nameplateDistance", NULL, (Console::CVarFlags)1, "43", CVarHandler_NameplateDistance);
    Hooks::FrameScript::registerToken("nameplate", getTokenGuid, getTokenId);

    DetourAttach(&(LPVOID&)Unit_ShowNamePlate_orig, Unit_ShowNamePlate_hk);
    DetourAttach(&(LPVOID&)Unit_HideNamePlate_orig, Unit_HideNamePlate_hk);
    DetourAttach(&(LPVOID&)HideAllNamePlates_orig, HideAllNamePlates_hk);
    DetourAttach(&(LPVOID&)Unit_dtor_orig, Unit_dtor_hk);
}