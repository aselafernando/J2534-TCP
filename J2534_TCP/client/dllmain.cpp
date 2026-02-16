// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <mutex>

std::mutex ini_mutex;

ofstream outfile;
bool doLog = false;
std::string serverAddr = "127.0.0.1";
int serverPort = 2534;

#ifdef _WIN32
static std::string logFile = "C:\\temp\\j2534.txt";
#else
static std::string logFile = "/tmp/j2534.txt";
#endif

void readINI() {
    //ini_mutex.lock();
    /*
    mINI::INIFile file("./J2534.ini");
    mINI::INIStructure ini;
    file.read(ini);
    if (ini["client"].has("log")) {
        std::string& log = ini["client"]["log"];
        doLog = (log.compare("1") == 0);
    }
    else {
        ini["client"]["log"] = "0";
    }

    if (ini["client"].has("logFile") == false) {
#ifdef _WIN32
        ini["client"]["logFile"] = "C:\\temp\\j2534.txt";
#else
        ini["client"]["logFile"] = "/tmp/j2534.txt";
#endif
    }

    logFile = ini.get("client").get("logFile");

    if (ini["client"].has("port")) {
        std::string& port = ini["client"]["port"];
        if (std::stoi(port) > 0) {
            serverPort = std::stoi(port);
        }
    }
    else {
        ini["client"]["port"] = "2534";
    }

    if (ini["client"].has("server") == false) {
        ini["client"]["server"] = "127.0.0.1";
    }

    serverAddr = ini.get("client").get("server");

    //file.write(ini);
    */
    if (doLog) {
        if (!outfile.is_open())
            outfile.open(logFile, ios::out);
    }
    //ini_mutex.unlock();
}

#ifdef _WIN32

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            readINI();
            if (doLog) {
                outfile << "attach\n";
            }
            break;
        case DLL_PROCESS_DETACH:
            if (doLog) {
                outfile << "detatch\n";
                outfile.close();
            }
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

#else
/*
__attribute__((constructor)) void DllLoad() {
    readINI();
    if (doLog) {
        outfile.open(logFile, ios::out);
        outfile << "attach\n";
    }
}

__attribute__((destructor)) void DllUnload() {
    if (doLog) {
        outfile << "detatch\n";
        outfile.close();
    }
}
*/
#endif
