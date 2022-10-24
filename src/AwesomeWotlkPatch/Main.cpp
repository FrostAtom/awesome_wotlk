#include "Utils.h"
#include "Patch.h"
#include <Windows.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#define AWESOMEWOTLKLIB_DLL "AwesomeWotlkLib.dll"

static std::string s_appName;

const char* findGameClientExecutable()
{
    static const char* possibleNames[] = { "Wow.exe", "WowCircle.exe", "run.exe" };
    for (const char* name : possibleNames)
        if (std::filesystem::is_regular_file(name))
            return name;
    return NULL;
}

bool applyPatches(const char* path)
{
    std::vector<char> image;
    if (!readFile(path, image)) return false;
    for (auto& [virtualAddress, hexBytes] : s_patches) {
        unsigned offset = virtualAddress2RawOffset(image.data(), virtualAddress);
        if (!offset) {
            SetLastError(ERROR_INVALID_ADDRESS);
            return false;
        }
        std::vector<char> data;
        if (!convHexString2ByteArray(hexBytes, data) || data.empty()) {
            SetLastError(ERROR_BAD_ARGUMENTS);
            return false;
        }
        memcpy(&image[offset], &data[0], data.size());
    }
    return writeFile(path, image);
}

template<typename... Args>
void message(DWORD icon, const char* fmt, Args&&... args)
{
    MessageBoxA(NULL, std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)).c_str(), s_appName.c_str(), icon | MB_OK);
}

int main(int argc, char** argv)
{
    s_appName = std::filesystem::path(argv[0]).filename().string();

    const char* exePath = NULL;
    if (argc > 1)
        exePath = argv[1];
    else
        exePath = findGameClientExecutable();

    if (!exePath) {
        message(MB_ICONERROR,
            "World of Warcraft executable not found!\n"
            "You must do one of this action:\n"
            "- Move patcher to folder with Wow.exe\n"
            "- Drag and drop Wow.exe into {}\n"
            "- Use command line interface `{} <path>`",
            s_appName, s_appName);
        return 1;
    }

    if (!applyPatches(exePath)) {
        DWORD lastError = GetLastError();
        const char* msg = NULL;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);
        message(MB_ICONERROR,
            "Can't apply patch to {}\n"
            "{}",
            "Error code {}",
            exePath, msg, lastError);
        return 1;
    }
    
    std::filesystem::path libInGamePath = std::filesystem::path(exePath).parent_path() / AWESOMEWOTLKLIB_DLL;
    std::filesystem::path libInAppPath = std::filesystem::absolute(argv[0]).parent_path() / AWESOMEWOTLKLIB_DLL;
    if (!std::filesystem::is_regular_file(libInGamePath) && std::filesystem::is_regular_file(libInAppPath)) {
        std::error_code ec;
        std::filesystem::copy_file(libInAppPath, libInGamePath, ec);
    }

    if (std::filesystem::is_regular_file(libInGamePath)) {
        message(MB_ICONINFORMATION,
            "Patch succesfully applied\n"
            "Now you can enter the game"
        );
    } else {
        message(MB_ICONWARNING,
            "Patch succesfully applied\n"
            "But look like `" AWESOMEWOTLKLIB_DLL "` is missed\n"
            "Before entering game you must place it to game folder"
        );
    }

    return 0;
}