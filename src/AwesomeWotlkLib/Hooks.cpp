#include "Hooks.h"
#include <Windows.h>
#include <Detours/detours.h>
#include <string>
#include <vector>
#include <unordered_map>

struct CVarArgs {
    Console::CVar** dst;
    const char* name;
    const char* desc;
    Console::CVarFlags flags;
    const char* initialValue;
    Console::CVar::Handler_t func;
};

static std::vector<CVarArgs> s_customCVars;
void Hooks::FrameXML::registerCVar(Console::CVar** dst, const char* str, const char* desc, Console::CVarFlags flags, const char* initialValue, Console::CVar::Handler_t func)
{
    s_customCVars.push_back({ dst, str, desc, flags, initialValue, func });
}

static void(*CVars_Initialize_orig)() = (decltype(CVars_Initialize_orig))0x0051D9B0;
static void CVars_Initialize_hk()
{
    CVars_Initialize_orig();
    for (const auto& [dst, name, desc, flags, initialValue, func] : s_customCVars) {
        Console::CVar* cvar = Console::RegisterCVar(name, desc, flags, initialValue, func, 0, 0, 0, 0);
        if (dst) *dst = cvar;
    }
}


static std::vector<const char*> s_customEvents;
void Hooks::FrameXML::registerEvent(const char* str) { s_customEvents.push_back(str); }

static void (*FrameScript_FillEvents_orig)(const char** list, size_t count) = (decltype(FrameScript_FillEvents_orig))0x0081B5F0;
static void FrameScript_FillEvents_hk(const char** list, size_t count)
{
    std::vector<const char*> events;
    events.reserve(count + s_customEvents.size());
    events.insert(events.end(), &list[0], &list[count]);
    events.insert(events.end(), s_customEvents.begin(), s_customEvents.end());
    FrameScript_FillEvents_orig(events.data(), events.size());
}


static std::vector<lua_CFunction> s_customLuaLibs;
void Hooks::FrameXML::registerLuaLib(lua_CFunction func) { s_customLuaLibs.push_back(func); }

static void Lua_OpenFrameXMlApi_bulk()
{
    lua_State* L = GetLuaState();
    for (auto& func : s_customLuaLibs)
        func(L);
}

static void(*Lua_OpenFrameXMLApi_orig)() = (decltype(Lua_OpenFrameXMLApi_orig))0x00530F85;
static void __declspec(naked) Lua_OpenFrameXMLApi_hk()
{
    __asm {
        pushad;
        pushfd;
        call Lua_OpenFrameXMlApi_bulk;
        popfd;
        popad;
        ret;
    }
}

struct CustomTokenDetails {
    CustomTokenDetails() { memset(this, NULL, sizeof(*this)); }
    CustomTokenDetails(Hooks::FrameScript::TokenGuidGetter* getGuid, Hooks::FrameScript::TokenIdGetter* getId)
        : hasN(false), getGuid(getGuid), getId(getId)
    {}
    CustomTokenDetails(Hooks::FrameScript::TokenNGuidGetter* getGuid, Hooks::FrameScript::TokenIdNGetter* getId)
        : hasN(true), getGuidN(getGuid), getIdN(getId)
    {}

    bool hasN;
    union {
        Hooks::FrameScript::TokenGuidGetter* getGuid;
        Hooks::FrameScript::TokenNGuidGetter* getGuidN;
    };
    union {
        Hooks::FrameScript::TokenIdGetter* getId;
        Hooks::FrameScript::TokenIdNGetter* getIdN;
    };
};
static std::unordered_map<std::string, CustomTokenDetails> s_customTokens;
void Hooks::FrameScript::registerToken(const char* token, TokenGuidGetter* getGuid, TokenIdGetter* getId) { s_customTokens[token] = { getGuid, getId }; }
void Hooks::FrameScript::registerToken(const char* token, TokenNGuidGetter* getGuid, TokenIdNGetter* getId) { s_customTokens[token] = { getGuid, getId }; }

static DWORD_PTR GetGuidByKeyword_jmpbackaddr = 0;
static void GetGuidByKeyword_bulk(const char** stackStr, guid_t* guid)
{
    for (auto& [token, conv] : s_customTokens) {
        if (strncmp(*stackStr, token.data(), token.size()) == 0) {
            *stackStr += token.size();
            if (conv.hasN) {
                int n = gc_atoi(stackStr);
                *guid = n > 0 ? conv.getGuidN(n - 1) : 0;
            } else {
                *guid = conv.getGuid();
            }
            GetGuidByKeyword_jmpbackaddr = 0x0060AD57;
            return;
        }
    }
    GetGuidByKeyword_jmpbackaddr = 0x0060AD44;
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

        push GetGuidByKeyword_jmpbackaddr;
        ret;
    }
}

static char** (*GetKeywordsByGuid_orig)(guid_t* guid, size_t* size) = (decltype(GetKeywordsByGuid_orig))0x0060BB70;
static char** GetKeywordsByGuid_hk(guid_t* guid, size_t* size)
{
    char** buf = GetKeywordsByGuid_orig(guid, size);
    if (!buf) return buf;
    for (auto& [token, conv] : s_customTokens) {
        if (*size >= 8) break;
        if (conv.hasN) {
            int id = conv.getIdN(*guid);
            if (id >= 0)
                snprintf(buf[(*size)++], 32, "%s%d", token.c_str(), id + 1);
        } else {
            if (conv.getId(*guid))
                snprintf(buf[(*size)++], 32, "%s", token.c_str());
        }
    }
    return buf;
}

static std::vector<Hooks::DummyCallback_t> s_customOnUpdate;
void Hooks::FrameScript::registerOnUpdate(DummyCallback_t func) { s_customOnUpdate.push_back(func); }

static int(*FrameScript_FireOnUpdate_orig)(int a1, int a2, int a3, int a4) = (decltype(FrameScript_FireOnUpdate_orig))0x00495810;
static int FrameScript_FireOnUpdate_hk(int a1, int a2, int a3, int a4)
{
    for (auto func : s_customOnUpdate)
        func();
    return FrameScript_FireOnUpdate_orig(a1, a2, a3, a4);
}

static std::vector<Hooks::DummyCallback_t> s_glueXmlPostLoad;
void Hooks::GlueXML::registerPostLoad(DummyCallback_t func) { s_glueXmlPostLoad.push_back(func); }

static void LoadGlueXML_bulk() { for (auto func : s_glueXmlPostLoad) func(); }

static void (*LoadGlueXML_orig)() = (decltype(LoadGlueXML_orig))0x004DA9AC;
static void __declspec(naked) LoadGlueXML_hk()
{
    __asm {
        pop ebx;
        mov esp, ebp;
        pop ebp;

        pushad;
        pushfd;
        call LoadGlueXML_bulk;
        popfd;
        popad;
        ret;
    }
}


static std::vector<Hooks::DummyCallback_t> s_glueXmlCharEnum;
void Hooks::GlueXML::registerCharEnum(DummyCallback_t func) { s_glueXmlCharEnum.push_back(func); }

static void LoadCharacters_bulk()
{
    for (auto func : s_glueXmlCharEnum)
        func();
}

static void (*LoadCharacters_orig)() = (decltype(LoadCharacters_orig))0x004E47E5;
static void __declspec(naked) LoadCharacters_hk()
{
    __asm {
        add esp, 8;
        pop esi;

        pushad;
        pushfd;
        call LoadCharacters_bulk;
        popfd;
        popad;

        ret;
    }
}

void Hooks::initialize()
{
    DetourAttach(&(LPVOID&)CVars_Initialize_orig, CVars_Initialize_hk);
    DetourAttach(&(LPVOID&)FrameScript_FireOnUpdate_orig, FrameScript_FireOnUpdate_hk);
    DetourAttach(&(LPVOID&)FrameScript_FillEvents_orig, FrameScript_FillEvents_hk);
    DetourAttach(&(LPVOID&)Lua_OpenFrameXMLApi_orig, Lua_OpenFrameXMLApi_hk);
    DetourAttach(&(LPVOID&)GetGuidByKeyword_orig, GetGuidByKeyword_hk);
    DetourAttach(&(LPVOID&)GetKeywordsByGuid_orig, GetKeywordsByGuid_hk);
    DetourAttach(&(LPVOID&)LoadGlueXML_orig, LoadGlueXML_hk);
    DetourAttach(&(LPVOID&)LoadCharacters_orig, LoadCharacters_hk);
}