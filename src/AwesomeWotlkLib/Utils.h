#pragma once
#include <Windows.h>
#include <string>
#include <string_view>

std::string u16tou8(std::wstring_view u16);
std::string GetFromClipboardU8(HWND hwnd);
bool CopyToClipboardU8(const char* u8Str, HWND hwnd);