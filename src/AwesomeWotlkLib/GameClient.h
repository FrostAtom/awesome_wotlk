#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstdarg>
#include <functional>

/*
    Game client types/functions/bindings and other import
*/

// Types
struct lua_State;
struct WorldFrame;
struct Camera;
struct Object;

struct Frame {
    int gap[2];
    int luaRef;
};

using guid_t = uint64_t;
using lua_Number = double;

template <typename T> struct Vec2D { T x, y; };
template <typename T> struct Vec3D { T x, y, z; };
template <typename T> struct Vec4D { T x, y, z, o; };
using VecXYZ = Vec3D<float>;

struct ObjectVtbl {
    void* field0[8];
    void(__thiscall* GetHeadPosition)(Object* self, VecXYZ* pos);
};

struct ObjectEntry {
    guid_t guid;
    int type;
    int entry;
    float scaleX;
    int padding;
};
static_assert(sizeof(ObjectEntry) == 0x18);

struct Object {
    ObjectVtbl* vmt;
    int field4;
    ObjectEntry* entry;
    uint32_t gap[779];
    Frame* nameplate;
};

enum ObjectFlags : uint32_t {
    ObjectFlags_Unit = 0x8,
};

// Base
inline HWND GetGameWindow() { return *(HWND*)0x00D41620; }

namespace CGame {
    inline void __stdcall SetLastError(int code) { return ((decltype(&SetLastError))0x00771870)(code); }
}

inline int __stdcall gc_atoi(const char** str) { return ((decltype(&gc_atoi))0x76F190)(str); }
inline int gc_snprintf(char* buf, size_t size, const char* fmt, ...)
{
    if (!(buf && fmt)) {
        CGame::SetLastError(87);
        return 0;
    }

    va_list args;
    va_start(args, fmt);
    size_t size_ = size;
    char* buf_ = buf;
    const char* fmt_ = fmt;
    int result;
    _asm {
        pushad;
        pushfd;
        mov eax, args;
        mov esi, size_;
        mov edi, buf_;
        mov ecx, fmt_;
        mov ebx, 0x76F010;
        call ebx;
        mov result, eax;
        popad;
        popfd;
    }
    return result;
}

namespace RCString {
inline uint32_t __stdcall hash(const char* str) { return ((decltype(&hash))0x0076F640)(str); }
}

// ObjectMgr
namespace ObjectMgr {

inline int EnumObjects_internal(int(*func)(guid_t, void*), void* udata) { return ((decltype(&EnumObjects_internal))0x004D4B30)(func, udata); }

using EnumVisibleObject_func_t = std::function<bool(guid_t guid)>;
inline bool EnumObjects(EnumVisibleObject_func_t func)
{
    struct Wrapper {
        static int foo(guid_t guid, void* udata)
        {
            EnumVisibleObject_func_t& func = *(EnumVisibleObject_func_t*)udata;
            return func(guid) ? 1 : 0;
        }
    };
    return EnumObjects_internal(&Wrapper::foo, (void*)&func);
};

inline Object* Get(guid_t guid, ObjectFlags flags) { return ((Object*(*)(guid_t, ObjectFlags))0x004D4DB0)(guid, flags); }
inline void Guid2HexString(guid_t guid, char* buf) { return ((decltype(&Guid2HexString))0x0074D0D0)(guid, buf); }
inline guid_t HexString2Guid(const char* str) { return ((decltype(&HexString2Guid))0x0074D120)(str); }
inline guid_t GetGuidByUnitID(const char* unitId) { return ((decltype(&GetGuidByUnitID))0x0060C1C0)(unitId); }

inline guid_t String2Guid(const char* str)
{
    if (!str) return 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        return HexString2Guid(str);
    else
        return GetGuidByUnitID(str);
}

inline Object* Get(const char* str, ObjectFlags flags) { return Get(String2Guid(str), flags); }

inline int UnitRightClickByGuid(guid_t guid) { return ((decltype(&UnitRightClickByGuid))0x005277B0)(guid); }
inline int UnitLeftClickByGuid(guid_t guid) { return ((decltype(&UnitLeftClickByGuid))0x005274F0)(guid); }
inline void SetMouseoverByGuid(guid_t guid, guid_t prev) { return ((decltype(&SetMouseoverByGuid))0x0051F790)(guid, prev); }

}

namespace Console {
enum CVarFlags : uint32_t {
    CVarFlags_ReadOnly = 0x4,
    CVarFlags_CheckTaint = 0x8,
    CVarFlags_HideFromUser = 0x40,
    CVarFlags_ReadOnlyForUser = 0x100,

};

struct CVar {
    using Handler_t = int(*)(CVar* cvar, const char* prevVal, const char* newVal, void* userData);

    uint32_t hash;
    uint32_t gap4[4];
    const char* name;
    uint32_t field18;
    CVarFlags flags;
    uint32_t field20;
    uint32_t field24;
    const char* vStr;
    uint32_t field2C[5];
    uint32_t vBool;
    uint32_t gap44[9];
    Handler_t handler;
    void* userData;
};
static_assert(sizeof(CVar) == 0x70);

inline CVar* RegisterCVar(const char* name, const char* desc, unsigned flags, const char* defaultVal, CVar::Handler_t callback, int a6, int a7, int a8, int a9) { return ((decltype(&RegisterCVar))0x00767FC0)(name, desc, flags, defaultVal, callback, a6, a7, a8, a9); };
inline CVar* GetCVar(const char* name) { return ((decltype(&GetCVar))0x00767460)(name); }
}

inline lua_State* GetLuaState() { return ((decltype(&GetLuaState))0x00817DB0)(); }
inline int GetLuaRefErrorHandler() { return *(int*)0x00AF576C; }

// CFrame
namespace CFrame {
inline int __fastcall GetRefTable(Frame* frame) { return ((decltype(&GetRefTable))0x00488380)(frame); }
}

// FrameScript
namespace FrameScript {
struct Event {
    uint32_t hash;
    uint32_t gap4[4];
    const char* name;
    uint32_t gap18[12];
    uint32_t field48;
    uint32_t field4C;
    uint32_t field50;
};

struct EventList {
    size_t reserve;
    size_t size;
    Event** buf;
};

struct UnkContainer;

inline UnkContainer* GetUnkContainer() { return (UnkContainer*)0x00D3F7A8; }
inline Event* __fastcall FindEvent(UnkContainer* This, void* edx, const char* eventName) { return ((decltype(&FindEvent))0x004BC410)(This, edx, eventName); }
inline EventList* GetEventList() { return (EventList*)0x00D3F7D0; }
inline void FireEvent_inner(int eventId, lua_State* L, int nargs) { return ((decltype(&FireEvent_inner))0x0081AA00)(eventId, L, nargs); };
inline void vFireEvent(int eventId, const char* format, va_list args) { return ((decltype(&vFireEvent))0x0081AC90)(eventId, format, args); }

inline int GetEventIdByName(const char* eventName)
{
    EventList* eventList = GetEventList();
    if (eventList->size == 0)
        return -1;

    uint32_t hash = RCString::hash(eventName);
    for (size_t i = 0; i < eventList->size; i++) {
        Event* event = eventList->buf[i];
        if (event && event->hash == hash && (event->name == eventName || (strcmp(event->name, eventName) == 0)))
            return i;
    }
    return -1;
}

inline const char* GetEventNameById(unsigned idx)
{
    EventList* eventList = GetEventList();
    if (eventList->size == 0 || eventList->size < idx)
        return NULL;

    Event* event = eventList->buf[idx];
    return event ? event->name : NULL;
}

inline void FireEvent(const char* eventName, const char* format, ...)
{
    int eventId = GetEventIdByName(eventName);
    if (eventId == -1) return;

    va_list args;
    va_start(args, format);
    vFireEvent(eventId, format, args);
}
}

// WorldFrame & Camera
struct CameraVtbl;

struct Camera {
    CameraVtbl* vmt;
    uint32_t field4;
    VecXYZ pos;
    uint32_t gap14[11];
    float fovInRadians;
};

inline Camera* GetActiveCamera() { return ((decltype(&GetActiveCamera))0x004F5960)(); }
inline WorldFrame* GetWorldFrame() { return *(WorldFrame**)0x00B7436C; }
inline int __fastcall WorldFrame_3Dto2D(WorldFrame* This, void* edx, VecXYZ* pos3d, VecXYZ* pos2d, uint32_t* flags) { return ((decltype(&WorldFrame_3Dto2D))0x004F6D20)(This, edx, pos3d, pos2d, flags); }
inline void WorldFrame_PercToScreenPos(float x, float y, float* resX, float* resY)
{
    float screenHeightAptitude = *(float*)0x00AC0CBC;
    float someVal = *(float*)0x00AC0CB4;
    *resX = (x * (screenHeightAptitude * 1024.f)) / someVal;
    *resY = (y * (screenHeightAptitude * 1024.f)) / someVal;
}


// Lua
#define lua_pop(L,n)		lua_settop(L, -(n)-1)
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)
#define lua_pushcfunction(L, f) lua_pushcclosure(L, f, 0);
#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))
#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)
#define lua_newtable(L) lua_createtable(L, 0, 0)

#define LUA_TNONE		(-1)
#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


using lua_CFunction = int(*)(lua_State*);
typedef struct luaL_Reg {
    const char* name;
    lua_CFunction func;
} luaL_Reg;

inline void luaL_checktype(lua_State* L, int idx, int t) { return ((decltype(&luaL_checktype))0x0084F960)(L, idx, t); }
inline const char* luaL_checkstring(lua_State* L, int idx) { return ((decltype(&luaL_checkstring))0x0084F9F0)(L, idx); }
inline void* lua_touserdata(lua_State* L, int idx) { return ((decltype(&lua_touserdata))0x0084E1C0)(L, idx); }
inline void lua_pushstring(lua_State* L, const char* str) { return ((decltype(&lua_pushstring))0x0084E350)(L, str); }
inline void lua_pushvalue(lua_State* L, int idx) { return ((decltype(&lua_pushvalue))0x0084DE50)(L, idx); }
inline void lua_pushnumber(lua_State* L, lua_Number v) { return ((decltype(&lua_pushnumber))0x0084E2A0)(L, v); }
inline void lua_pushcclosure(lua_State* L, lua_CFunction func, int c) { return ((decltype(&lua_pushcclosure))0x0084E400)(L, func, c); }
inline void lua_pushnil(lua_State* L) { return ((decltype(&lua_pushnil))0x0084E280)(L); }
inline void lua_rawseti(lua_State* L, int idx, int pos) { return ((decltype(&lua_rawseti))0x0084EA00)(L, idx, pos); }
inline void lua_rawgeti(lua_State* L, int idx, int pos) { return ((decltype(&lua_rawgeti))0x0084E670)(L, idx, pos); }
inline void lua_rawset(lua_State* L, int idx) { return ((decltype(&lua_rawset))0x0084E970)(L, idx); }
inline void lua_setfield(lua_State* L, int idx, const char* str) { return ((decltype(&lua_setfield))0x0084E900)(L, idx, str); }
inline void lua_getfield(lua_State* L, int idx, const char* str) { return ((decltype(&lua_getfield))0x0084E590)(L, idx, str); }
inline int lua_next(lua_State* L, int idx) { return ((decltype(&lua_next))0x0084EF50)(L, idx); }
inline void lua_insert(lua_State* L, int idx) { return ((decltype(&lua_insert))0x0084DCC0)(L, idx); }
inline int lua_gettop(lua_State* L) { return ((decltype(&lua_gettop))0x0084DBD0)(L); }
inline void lua_settop(lua_State* L, int idx) { return ((decltype(&lua_settop))0x0084DBF0)(L, idx); }
inline int lua_objlen(lua_State* L, int idx) { return ((decltype(&lua_objlen))0x0084E150)(L, idx); }
inline int lua_type(lua_State* L, int idx) { return ((decltype(&lua_type))0x0084DEB0)(L, idx); }
inline int lua_pcall(lua_State* L, int argn, int retn, int eh) { return ((decltype(&lua_pcall))0x0084EC50)(L, argn, retn, eh); }
inline int lua_GetParamValue(lua_State* L, int idx, int default_) { return ((decltype(&lua_GetParamValue))0x00815500)(L, idx, default_); }
inline void lua_createtable(lua_State* L, int narr, int nrec) { return ((decltype(&lua_createtable))0x0084E6E0)(L, narr, nrec); }

inline void lua_wipe(lua_State* L, int idx)
{
    if (idx < 0) idx = lua_gettop(L) - (idx + 1);
    lua_pushnil(L); // nil
    while (lua_next(L, idx)) { // key, value
        lua_pop(L, 1); // key
        lua_pushnil(L); // key, nil
        lua_rawset(L, idx); //
        lua_pushnil(L); // key, nil
    }
}

inline void lua_pushguid(lua_State* L, guid_t guid)
{
    char buf[24];
    ObjectMgr::Guid2HexString(guid, buf);
    lua_pushstring(L, buf);
}

inline Frame* lua_toframe(lua_State* L, int idx)
{
    __asm {
        mov esi, L;
        push idx;
        mov eax, 0x004A81B0;
        call eax;
        ret;
    }
}

inline Frame* lua_toframe_silent(lua_State* L, int idx)
{
    lua_rawgeti(L, idx, 0);
    Frame* frame = (Frame*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return frame;
}

inline void lua_pushframe(lua_State* L, Frame* frame)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, CFrame::GetRefTable(frame));
}