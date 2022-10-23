#include <Windows.h>
#include <Detours/detours.h>
#include "NamePlates.h"


int __stdcall DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch(reason) {
    case DLL_PROCESS_ATTACH: {
        DetourTransactionBegin();
        NamePlates::Initialize();
        DetourTransactionCommit();


        *(DWORD*)0x00D415B8 = 1;
        *(DWORD*)0x00D415BC = 0x7FFFFFFF;
        break;
    }
    case DLL_PROCESS_DETACH: {
        // TODO
        break;
    }
    }
    return 1;
}