#include "CommandLine.h"
#include "Hooks.h"
#include "GameClient.h"
#include "Utils.h"
#include <shellapi.h>

static std::vector<std::string> s_commandLine;


static void gluexml_charenum()
{
    static bool s_once = false;
    if (s_once) return;
    s_once = true;

    const char* character = NULL;
    for (int i = 1; i < s_commandLine.size(); i++) {
        if (strcmp(s_commandLine[i].c_str(), "-character") == 0) {
            if ((++i) < s_commandLine.size())
                character = s_commandLine[i].c_str();
        }
    }

    if (character) {
        LoginUI::CharVector* chars = LoginUI::GetChars();
        for (int i = 0; i < chars->size; i++) {
            if (strcmp(chars->buf[i].data.name, character) == 0) {
                LoginUI::EnterWorld(i);
                break;
            }
        }
    }
}

static void gluexml_postload()
{
    static bool s_once = false;
    if (s_once) return;
    s_once = true;

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
}

void CommandLine::initialize()
{
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 0; i < argc; i++)
        s_commandLine.emplace_back(u16tou8(argv[i]));

    Hooks::GlueXML::registerCharEnum(gluexml_charenum);
    Hooks::GlueXML::registerPostLoad(gluexml_postload);
}