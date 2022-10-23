#pragma once
#include "GameClient.h"

bool lua_callfunction(lua_State* L, int argn, int retn);
bool lua_getnamespace(lua_State* L);
void lua_openlibnameplates(lua_State* L);