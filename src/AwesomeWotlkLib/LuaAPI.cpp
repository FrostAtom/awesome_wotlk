#include "LuaAPI.h"

extern bool s_useGuidAsUnitID;

static int lua_UnitRightClick(lua_State* L)
{
    const char* str = luaL_checkstring(L, 1);
    guid_t guid = ObjectMgr::String2Guid(str);
    if (!guid) return 0;
    *(void**)0x00D4139C = 0;
    ObjectMgr::UnitRightClickByGuid(guid);
    return 0;
}

static int lua_UnitLeftClick(lua_State* L)
{
    const char* str = luaL_checkstring(L, 1);
    guid_t guid = ObjectMgr::String2Guid(str);
    if (!guid) return 0;
    *(void**)0x00D4139C = 0;
    ObjectMgr::UnitLeftClickByGuid(guid);
    return 0;
}

static int lua_UnitMouseover(lua_State* L)
{
    guid_t guid = 0;
    if (lua_gettop(L) > 0) {
        const char* str = luaL_checkstring(L, 1);
        guid = ObjectMgr::String2Guid(str);
    }
    ObjectMgr::SetMouseoverByGuid(guid, 0);
    return 0;
}

static int lua_UnitDisplayCoords(lua_State* L)
{
    const char* str = luaL_checkstring(L, 1);
    Object* unit = ObjectMgr::Get(str, ObjectFlags_Unit);
    if (!unit) return 0;

    VecXYZ pos3d, pos2d;
    unit->vmt->GetHeadPosition(unit, &pos3d);
    if (!WorldFrame_3Dto2D(GetWorldFrame(), NULL, &pos3d, &pos2d, NULL)) return 0;
    WorldFrame_PercToScreenPos(pos2d.x, pos2d.y, &pos2d.x, &pos2d.y);
    lua_pushnumber(L, pos2d.x);
    lua_pushnumber(L, pos2d.y);
    return 2;
}

static int lua_GetVisibleUnits(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE); // tbl
    lua_settop(L, 1);

    ObjectMgr::EnumObjects([L](uint64_t guid) {
        if (ObjectMgr::Get(guid, ObjectFlags_Unit)) {
            char buf[24];
            snprintf(buf, std::size(buf), "0x%016llX", guid);
            lua_pushstring(L, buf); // tbl, v
            lua_rawseti(L, -2, lua_objlen(L, -2) + 1); // tbl
        }
        return true;
    });
    return 1;
}

static int lua_loadlib_nameplates(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);

    lua_pushcfunction(L, lua_UnitRightClick);
    lua_setfield(L, -2, "UnitRightClick");
    lua_pushcfunction(L, lua_UnitLeftClick);
    lua_setfield(L, -2, "UnitLeftClick");
    lua_pushcfunction(L, lua_UnitMouseover);
    lua_setfield(L, -2, "UnitMouseover");
    lua_pushcfunction(L, lua_UnitDisplayCoords);
    lua_setfield(L, -2, "UnitDisplayCoords");
    lua_pushcfunction(L, lua_GetVisibleUnits);
    lua_setfield(L, -2, "GetVisibleUnits");

    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "nameplate_main");

    return 0;
}

bool lua_callfunction(lua_State* L, int argn, int retn)
{
    if (lua_pcall(L, argn, retn, 0) == 0)
        return true;

    lua_rawgeti(L, LUA_REGISTRYINDEX, GetLuaRefErrorHandler());
    if (lua_pcall(L, 1, 0, 0) != 0)
        lua_pop(L, 1);
    return false;
}

bool lua_getnamespace(lua_State* L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "nameplate_main");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void lua_openlibnameplates(lua_State* L)
{
    lua_pushcfunction(L, lua_loadlib_nameplates);
    lua_setglobal(L, "loadlib_nameplates");
}