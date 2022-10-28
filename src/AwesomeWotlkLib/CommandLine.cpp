#include "CommandLine.h"
#include "Hooks.h"
#include "GameClient.h"
#include "Utils.h"
#include <shellapi.h>

static std::vector<std::string> s_commandLine;


static int lua_gluexml_postload(lua_State* L)
{
    const char* login = NULL, *password = NULL;
    for (int i = 1; i < s_commandLine.size(); i++) {
        if (strcmp(s_commandLine[i].c_str(), "-login") == 0) {
            if ((++i) < s_commandLine.size())
                login = s_commandLine[i].c_str();
        } else if (strcmp(s_commandLine[i].c_str(), "-password") == 0) {
            if ((++i) < s_commandLine.size())
                password = s_commandLine[i].c_str();
        }
    }
    if (login && password) NetClient::Login(login, password);
    return 0;
}

void CommandLine::initialize()
{
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 0; i < argc; i++)
        s_commandLine.emplace_back(u16tou8(argv[i]));

    if (std::find(s_commandLine.begin(), s_commandLine.end(), "-break") != s_commandLine.end()) {
        system("pause");
        FreeConsole();
    }

    Hooks::GlueXML::registerPostLoad(lua_gluexml_postload);
}