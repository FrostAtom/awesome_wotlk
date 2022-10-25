#include "Utils.h"

std::string GetFromClipboardU8(HWND hwnd)
{
    if (!OpenClipboard(hwnd))
        return {};
    HANDLE hMem = GetClipboardData(CF_UNICODETEXT);
    if (!hMem) {
    on_fail:
        CloseClipboard();
        return {};
    }
    const wchar_t* utf16 = (const wchar_t*)GlobalLock(hMem);
    if (!utf16) goto on_fail;
    int utf16Length = wcslen(utf16) + 1;
    int utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16, utf16Length, NULL, 0, NULL, NULL);
    if (utf8Length == 0) {
        GlobalUnlock(hMem);
        goto on_fail;
    }

    std::string utf8;
    utf8.resize(utf8Length);
    int ret = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16, utf16Length, &utf8[0], utf8Length, NULL, NULL);


    GlobalUnlock(hMem);
    CloseClipboard();
    return utf8;
}

bool CopyToClipboardU8(const char* u8Str, HWND hwnd)
{
    if (!u8Str || !u8Str[0]) { // just empty
        if (!OpenClipboard(hwnd))
            return false;
        bool result = EmptyClipboard();
        CloseClipboard();
        return result;
    }
    int u8CharsLen = strlen(u8Str) + 1;
    int wCharsLen = MultiByteToWideChar(CP_UTF8, 0, u8Str, u8CharsLen, 0, 0);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * wCharsLen);
    if (!hMem)
        return false;
    wchar_t* cbBuf = (wchar_t*)GlobalLock(hMem);
    if (!cbBuf) {
    on_fail:
        GlobalFree(hMem);
        return false;
    }

    MultiByteToWideChar(CP_UTF8, 0, u8Str, u8CharsLen, cbBuf, wCharsLen);
    cbBuf[wCharsLen] = L'\0';
    GlobalUnlock(hMem);

    if (!OpenClipboard(hwnd))
        goto on_fail;
    if (!EmptyClipboard()) {
        CloseClipboard();
        goto on_fail;
    }

    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    return true;
}
