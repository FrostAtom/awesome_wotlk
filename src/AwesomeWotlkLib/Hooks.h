#pragma once
#include "GameClient.h"
#include <eventpp/callbacklist.h>

namespace Hooks {

namespace FrameScript {
using TokenGuidGetter = guid_t();
using TokenNGuidGetter = guid_t(int);
using TokenIdGetter = bool(guid_t);
using TokenIdNGetter = int(guid_t);

// Alone tokens like player, target, focus
void registerToken(const char* token, TokenGuidGetter* getGuid, TokenIdGetter* getId);
// One more tokens like party1, raid1, arena1
void registerToken(const char* token, TokenNGuidGetter* getGuid, TokenIdNGetter* getId);
}

namespace FrameXML {
void registerEvent(const char* str);
void registerCVar(Console::CVar** dst, const char* str, const char* desc, Console::CVarFlags flags, const char* initialValue, Console::CVar::Handler_t func);
void registerLuaLib(lua_CFunction func);
}

void Initialize();

}