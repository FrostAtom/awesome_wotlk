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
struct Status;
struct Frame;
struct XMLObject;
struct Object;
struct ObjectVtbl;
struct ObjectEntry;
struct Unit;
struct UnitVtbl;
struct UnitEntry;
struct Player;
struct PlayerVtbl;
struct PlayerEntry;
using guid_t = uint64_t;
using lua_Number = double;

template <typename T> struct Vec2D { T x, y; };
template <typename T> struct Vec3D { T x, y, z; };
template <typename T> struct Vec4D { T x, y, z, o; };
struct VecXYZ : Vec3D<float> {

    inline VecXYZ operator-(const VecXYZ& r) { return { x - r.x, y - r.y, z - r.z }; }

    inline float distance(const VecXYZ& other)
    {
        VecXYZ diff = (*this) - other;
        return std::sqrtf(std::powf(diff.x, 2) + std::powf(diff.y, 2) + std::powf(diff.z, 2));
    }
};

enum UnitFlags : uint32_t {
    UNIT_FLAG_SERVER_CONTROLLED = 0x00000001,           // set only when unit movement is controlled by server - by SPLINE/MONSTER_MOVE packets, together with UNIT_FLAG_STUNNED; only set to units controlled by client; client function CGUnit_C::IsClientControlled returns false when set for owner
    UNIT_FLAG_NON_ATTACKABLE = 0x00000002,           // not attackable, set when creature starts to cast spells with SPELL_EFFECT_SPAWN and cast time, removed when spell hits caster, original name is UNIT_FLAG_SPAWNING. Rename when it will be removed from all scripts
    UNIT_FLAG_REMOVE_CLIENT_CONTROL = 0x00000004,           // This is a legacy flag used to disable movement player's movement while controlling other units, SMSG_CLIENT_CONTROL replaces this functionality clientside now. CONFUSED and FLEEING flags have the same effect on client movement asDISABLE_MOVE_CONTROL in addition to preventing spell casts/autoattack (they all allow climbing steeper hills and emotes while moving)
    UNIT_FLAG_PLAYER_CONTROLLED = 0x00000008,           // controlled by player, use _IMMUNE_TO_PC instead of _IMMUNE_TO_NPC
    UNIT_FLAG_RENAME = 0x00000010,
    UNIT_FLAG_PREPARATION = 0x00000020,           // don't take reagents for spells with SPELL_ATTR5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_UNK_6 = 0x00000040,
    UNIT_FLAG_NOT_ATTACKABLE_1 = 0x00000080,           // ?? (UNIT_FLAG_PLAYER_CONTROLLED | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_IMMUNE_TO_PC = 0x00000100,           // disables combat/assistance with PlayerCharacters (PC) - see Unit::IsValidAttackTarget, Unit::IsValidAssistTarget
    UNIT_FLAG_IMMUNE_TO_NPC = 0x00000200,           // disables combat/assistance with NonPlayerCharacters (NPC) - see Unit::IsValidAttackTarget, Unit::IsValidAssistTarget
    UNIT_FLAG_LOOTING = 0x00000400,           // loot animation
    UNIT_FLAG_PET_IN_COMBAT = 0x00000800,           // on player pets: whether the pet is chasing a target to attack || on other units: whether any of the unit's minions is in combat
    UNIT_FLAG_PVP_ENABLING = 0x00001000,           // changed in 3.0.3, now UNIT_BYTES_2_OFFSET_PVP_FLAG from UNIT_FIELD_BYTES_2
    UNIT_FLAG_SILENCED = 0x00002000,           // silenced, 2.1.1
    UNIT_FLAG_CANT_SWIM = 0x00004000,           // TITLE Can't Swim
    UNIT_FLAG_CAN_SWIM = 0x00008000,           // TITLE Can Swim DESCRIPTION shows swim animation in water
    UNIT_FLAG_NON_ATTACKABLE_2 = 0x00010000,           // removes attackable icon, if on yourself, cannot assist self but can cast TARGET_SELF spells - added by SPELL_AURA_MOD_UNATTACKABLE
    UNIT_FLAG_PACIFIED = 0x00020000,           // 3.0.3 ok
    UNIT_FLAG_STUNNED = 0x00040000,           // 3.0.3 ok
    UNIT_FLAG_IN_COMBAT = 0x00080000,
    UNIT_FLAG_ON_TAXI = 0x00100000,           // disable casting at client side spell not allowed by taxi flight (mounted?), probably used with 0x4 flag
    UNIT_FLAG_DISARMED = 0x00200000,           // 3.0.3, disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED = 0x00400000,
    UNIT_FLAG_FLEEING = 0x00800000,
    UNIT_FLAG_POSSESSED = 0x01000000,           // under direct client control by a player (possess or vehicle)
    UNIT_FLAG_UNINTERACTIBLE = 0x02000000,
    UNIT_FLAG_SKINNABLE = 0x04000000,
    UNIT_FLAG_MOUNT = 0x08000000,
    UNIT_FLAG_UNK_28 = 0x10000000,
    UNIT_FLAG_PREVENT_EMOTES_FROM_CHAT_TEXT = 0x20000000,   // Prevent automatically playing emotes from parsing chat text, for example "lol" in /say, ending message with ? or !, or using /yell
    UNIT_FLAG_SHEATHE = 0x40000000,
    UNIT_FLAG_IMMUNE = 0x80000000,           // Immune to damage

    UNIT_FLAG_DISALLOWED = (UNIT_FLAG_SERVER_CONTROLLED | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL |
    UNIT_FLAG_PLAYER_CONTROLLED | UNIT_FLAG_RENAME | UNIT_FLAG_PREPARATION | /* UNIT_FLAG_UNK_6 | */
        UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_LOOTING | UNIT_FLAG_PET_IN_COMBAT | UNIT_FLAG_PVP_ENABLING |
        UNIT_FLAG_SILENCED | UNIT_FLAG_NON_ATTACKABLE_2 | UNIT_FLAG_PACIFIED | UNIT_FLAG_STUNNED |
        UNIT_FLAG_IN_COMBAT | UNIT_FLAG_ON_TAXI | UNIT_FLAG_DISARMED | UNIT_FLAG_CONFUSED | UNIT_FLAG_FLEEING |
        UNIT_FLAG_POSSESSED | UNIT_FLAG_SKINNABLE | UNIT_FLAG_MOUNT | UNIT_FLAG_UNK_28 |
        UNIT_FLAG_PREVENT_EMOTES_FROM_CHAT_TEXT | UNIT_FLAG_SHEATHE | UNIT_FLAG_IMMUNE), // SKIP

    UNIT_FLAG_ALLOWED = (0xFFFFFFFF & ~UNIT_FLAG_DISALLOWED) // SKIP
};

struct ObjectEntry {
    guid_t guid;
    int type;
    int entry;
    float scaleX;
    int padding;
};
static_assert(sizeof(ObjectEntry) == 0x18);

struct UnitEntry : ObjectEntry {
    guid_t charm;
    guid_t summon;
    guid_t critter;
    guid_t charmedBy;
    guid_t summonedBy;
    guid_t createdBy;
    guid_t target;
    guid_t channelObject;
    uint32_t channelSpell;
    uint32_t bytes0;
    uint32_t health;
    uint32_t power[7];
    uint32_t maxHealth;
    uint32_t maxPower[7];
    uint32_t powerRegenFlatModifier[7];
    uint32_t powerRegenInterruptedFlatModifier[7];
    uint32_t level;
    uint32_t factionTemplate;
    uint32_t virtualItemSlotId[3];
    uint32_t flags;
    uint32_t flags2;

    char gap[0x15C];
};
static_assert(sizeof(UnitEntry) == 0x250);

struct PlayerQuest {
    int a1, a2, a3, a4, a5;
};
static_assert(sizeof(PlayerQuest) == 0x14);

struct PlayerVisibleItem {
    int entryId;
    int enchant;
};
static_assert(sizeof(PlayerVisibleItem) == 0x8);

struct PlayerEntry : UnitEntry {
    UnitEntry unit;
    guid_t duelArbiter;
    uint32_t flags;
    uint32_t guildId, guildRank;
    uint32_t bytes1, bytes2, bytes3;
    uint32_t duelTeam;
    uint32_t guildTimestamp;
    PlayerQuest quests[25];
    PlayerVisibleItem visibleItems[19];
};

struct ObjectVtbl {};

struct UnitVtbl {
    DWORD gap0[11];
    void(__thiscall* GetPosition)(Unit* self, VecXYZ* pos);
};

struct PlayerVtbl {};

struct Object {
    ObjectVtbl* vmt;
    int field4;
    ObjectEntry* entry;
};

struct Unit {
    UnitVtbl* vmt;
    int field4;
    UnitEntry* entry;
    uint32_t gap[779];
    Frame* nameplate;

    inline Object* ToObject() { return (Object*)this; }
};

struct Player {
    PlayerVtbl* vmt;
    int field4;
    PlayerEntry* entry;

    inline Unit* ToUnit() { return (Unit*)this; }
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

inline bool IsInWorld() { return *(char*)0x00BD0792; }

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

inline Player* GetPlayer() { return ((decltype(&GetPlayer))0x004038F0)(); }
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
inline guid_t GetTargetGuid() { return *(guid_t*)0x00BD07B0; }

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
inline CVar* FindCVar(const char* name) { return ((decltype(&FindCVar))0x00767440)(name); }
inline char SetCVarValue(CVar* self, const char* value, int a3, int a4, int a5, int a6)
{
    return (((char(__thiscall*)(CVar*, const char*, int, int, int, int))0x007668C0))(self, value, a3, a4, a5, a6);
}
}

inline lua_State* GetLuaState() { return ((decltype(&GetLuaState))0x00817DB0)(); }
inline int GetLuaRefErrorHandler() { return *(int*)0x00AF576C; }

// CFrame
namespace CFrame {
inline int GetRefTable(Frame* frame) { return ((int(__thiscall*)(Frame*))0x00488380)(frame); }
inline Frame* Create(XMLObject* xml, Frame* parent, Status* status) { return ((decltype(&Create))0x00812FA0)(xml, parent, status); }
inline void SetFrameLevel(Frame* self, int level, int a3) { ((void(__thiscall*)(Frame*, int, int))0x004910A0)(self, level, a3); }
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

// NetClient
namespace NetClient {
inline void Login(const char* login, const char* password) { return ((decltype(&Login))0x004D8A30)(login, password); }
}

// LoginUI
namespace LoginUI {

#pragma pack(push, 1)
struct CharData {
    guid_t guid;
    char name[48];
    int map;
    int zone;
    int guildId;
    VecXYZ pos;
    int displayInfoId[23];
    int inventoryType[23];
    int enchantVisual[23];
    int petDisplayId;
    int petLevel;
    int petFamily;
    int flags;
    int charCustomizeFlags;
    char race;
    char class_;
    char gender;;
    char skin;
    char face;
    char hairStyle;
    char hairColor;
    char facialColor;
    char level;
    char firstLogin;
    char gap[6];
};
#pragma pack(pop)
static_assert(sizeof(CharData) == 0x188, "struct CharData corrupted");

struct CharVectorEntry {
    CharData data;
    // Note: It's not all fields
};

struct CharVector {
    int reserved;
    int size;
    CharVectorEntry* buf;
    int fieldC;
};

inline CharVector* GetChars() { return (CharVector*)0x00B6B238; }
inline void SelectCharacter(int idx)
{
    *(int*)0x00AC436C = idx;
    ((void(*)())0x004E3CD0)();
}

inline void EnterWorld(int idx)
{
    *(int*)0x00AC436C = idx;
    ((void(*)())0x004D9BD0)();
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

// XML
struct __declspec(novtable) XMLObject {
    uint32_t gap0[0x38 / 4];

    inline XMLObject(int a1, const char* parentName) { ((XMLObject * (__thiscall*)(XMLObject*, int, const char*))0x00814AD0)(this, a1, parentName); }
    inline void setValue(const char* key, const char* value) { ((void(__thiscall*)(XMLObject*, const char*, const char*))0x814C40)(this, key, value); }
};

// Lua
#define lua_pop(L,n)		lua_settop(L, -(n)-1)
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isuserdata(L,n) (lua_type(L,n) == LUA_TLIGHTUSERDATA) || (lua_type(L, n) == LUA_TUSERDATA)
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
#define luaL_checkstring(L, i) luaL_checklstring(L, i, NULL)

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
inline const char* luaL_checklstring(lua_State* L, int idx, size_t* len) { return ((decltype(&luaL_checklstring))0x0084F9F0)(L, idx, len); }
inline lua_Number luaL_checknumber(lua_State* L, int idx) { return ((decltype(&luaL_checknumber))0x84FAB0)(L, idx); }
inline void* lua_touserdata(lua_State* L, int idx) { return ((decltype(&lua_touserdata))0x0084E1C0)(L, idx); }
inline void lua_pushstring(lua_State* L, const char* str) { return ((decltype(&lua_pushstring))0x0084E350)(L, str); }
inline void lua_pushvalue(lua_State* L, int idx) { return ((decltype(&lua_pushvalue))0x0084DE50)(L, idx); }
inline void lua_pushnumber(lua_State* L, lua_Number v) { return ((decltype(&lua_pushnumber))0x0084E2A0)(L, v); }
inline void lua_pushcclosure(lua_State* L, lua_CFunction func, int c) { return ((decltype(&lua_pushcclosure))0x0084E400)(L, func, c); }
inline void lua_pushnil(lua_State* L) { return ((decltype(&lua_pushnil))0x0084E280)(L); }
inline void lua_rawseti(lua_State* L, int idx, int pos) { return ((decltype(&lua_rawseti))0x0084EA00)(L, idx, pos); }
inline void lua_rawgeti(lua_State* L, int idx, int pos) { return ((decltype(&lua_rawgeti))0x0084E670)(L, idx, pos); }
inline void lua_rawset(lua_State* L, int idx) { return ((decltype(&lua_rawset))0x0084E970)(L, idx); }
inline void lua_rawget(lua_State* L, int idx) { return ((decltype(&lua_rawget))0x0084E600)(L, idx); }
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
inline void* lua_newuserdata(lua_State* L, size_t size) { return ((decltype(&lua_newuserdata))0x0084F0F0)(L, size); }
inline int lua_setmetatable(lua_State* L, int idx) { return ((decltype(&lua_setmetatable))0x0084EA90)(L, idx); }

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