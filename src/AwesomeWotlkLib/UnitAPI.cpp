#include "UnitAPI.h"
#include "GameClient.h"
#include "Hooks.h"


static int lua_UnitIsControlled(lua_State* L)
{
    Unit* unit = (Unit*)ObjectMgr::Get(luaL_checkstring(L, 1), ObjectFlags_Unit);
    if (!unit || !(unit->entry->flags & (UNIT_FLAG_FLEEING | UNIT_FLAG_CONFUSED | UNIT_FLAG_STUNNED | UNIT_FLAG_PACIFIED)))
        return 0;
    lua_pushnumber(L, 1);
    return 1;
}

static int lua_UnitIsDisarmed(lua_State* L)
{
    Unit* unit = (Unit*)ObjectMgr::Get(luaL_checkstring(L, 1), ObjectFlags_Unit);
    if (!unit || !(unit->entry->flags & UNIT_FLAG_DISARMED))
        return 0;
    lua_pushnumber(L, 1);
    return 1;
}

static int lua_UnitIsSilenced(lua_State* L)
{
    Unit* unit = (Unit*)ObjectMgr::Get(luaL_checkstring(L, 1), ObjectFlags_Unit);
    if (!unit || !(unit->entry->flags & UNIT_FLAG_SILENCED))
        return 0;
    lua_pushnumber(L, 1);
    return 1;
}

static int lua_openunitlib(lua_State* L)
{
    luaL_Reg funcs[] = {
        { "UnitIsControlled", lua_UnitIsControlled },
        { "UnitIsDisarmed", lua_UnitIsDisarmed },
        { "UnitIsSilenced", lua_UnitIsSilenced },
    };

    for (auto& [name, func] : funcs) {
        lua_pushcfunction(L, func);
        lua_setglobal(L, name);
    }

    return 0;
}

void UnitAPI::initialize()
{
    Hooks::FrameXML::registerLuaLib(lua_openunitlib);
}