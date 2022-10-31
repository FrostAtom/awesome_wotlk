#include "Inventory.h"
#include "Hooks.h"
#include "GameClient.h"


static int lua_GetInventoryItemTransmog(lua_State* L)
{
    const char* unit = luaL_checkstring(L, 1);
    int id = luaL_checknumber(L, 2) - 1;
    Player* player = (Player*)ObjectMgr::Get(unit, ObjectFlags_Unit);
    if (!player || (id < 0 || id >= 19)) return 0;
    PlayerEntry* entry = (PlayerEntry*)player->entry;
    lua_pushnumber(L, entry->visibleItems[id].entryId);
    lua_pushnumber(L, entry->visibleItems[id].enchant);
    return 2;
}

static int lua_openlibinventory(lua_State* L)
{
    lua_pushcfunction(L, lua_GetInventoryItemTransmog);
    lua_setglobal(L, "GetInventoryItemTransmog");
    return 0;
}

void Inventory::initialize()
{
    Hooks::FrameXML::registerLuaLib(lua_openlibinventory);
}