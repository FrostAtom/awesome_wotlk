#pragma once
#include <Windows.h>
#include <string>

std::string GetFromClipboardU8(HWND hwnd);
bool CopyToClipboardU8(const char* u8Str, HWND hwnd);