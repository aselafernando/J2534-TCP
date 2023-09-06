#ifdef _WIN32
#define WINDOWS_IGNORE_PACKING_MISMATCH
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "J2534_V0404.h"
#include "J2534.h"

_PassThruConnect PassThruConnect = NULL;
_PassThruDisconnect PassThruDisconnect = NULL;
_PassThruReadMsgs PassThruReadMsgs = NULL;
_PassThruWriteMsgs PassThruWriteMsgs = NULL;
_PassThruStartPeriodicMsg PassThruStartPeriodicMsg = NULL;
_PassThruStopPeriodicMsg PassThruStopPeriodicMsg = NULL;
_PassThruStartMsgFilter PassThruStartMsgFilter = NULL;
_PassThruStopMsgFilter PassThruStopMsgFilter = NULL;
_PassThruSetProgrammingVoltage PassThruSetProgrammingVoltage = NULL;
_PassThruReadVersion PassThruReadVersion = NULL;
_PassThruGetLastError PassThruGetLastError = NULL;
_PassThruIoctl PassThruIoctl = NULL;
_PassThruOpen PassThruOpen = NULL;
_PassThruClose PassThruClose = NULL;
_PassThruReset PassThruReset = NULL;
_PassThruGetLastSocketError PassThruGetLastSocketError = NULL;

#ifdef _WIN32
static HINSTANCE hJ2534 = 0;
#else
static void* hJ2534 = NULL;
#endif

using namespace std;

int loadDLL() {
#ifdef _WIN32
    hJ2534 = LoadLibrary(L"J2534.dll");

    if (hJ2534 == NULL) {
        return GetLastError();
    }
    if ((PassThruConnect = (_PassThruConnect)GetProcAddress(hJ2534, "PassThruConnect")) == NULL) {
        return GetLastError();
    }
    if ((PassThruDisconnect = (_PassThruDisconnect)GetProcAddress(hJ2534, "PassThruDisconnect")) == NULL) {
        return GetLastError();
    }
    if ((PassThruReadMsgs = (_PassThruReadMsgs)GetProcAddress(hJ2534, "PassThruReadMsgs")) == NULL) {
        return GetLastError();
    }
    if ((PassThruWriteMsgs = (_PassThruWriteMsgs)GetProcAddress(hJ2534, "PassThruWriteMsgs")) == NULL) {
        return GetLastError();
    }
    if ((PassThruStartPeriodicMsg = (_PassThruStartPeriodicMsg)GetProcAddress(hJ2534, "PassThruStartPeriodicMsg")) == NULL) {
        return GetLastError();
    }
    if ((PassThruStopPeriodicMsg = (_PassThruStopPeriodicMsg)GetProcAddress(hJ2534, "PassThruStopPeriodicMsg")) == NULL) {
        return GetLastError();
    }
    if ((PassThruStartMsgFilter = (_PassThruStartMsgFilter)GetProcAddress(hJ2534, "PassThruStartMsgFilter")) == NULL) {
        return GetLastError();
    }
    if ((PassThruStopMsgFilter = (_PassThruStopMsgFilter)GetProcAddress(hJ2534, "PassThruStopMsgFilter")) == NULL) {
        return GetLastError();
    }
    if ((PassThruSetProgrammingVoltage = (_PassThruSetProgrammingVoltage)GetProcAddress(hJ2534, "PassThruSetProgrammingVoltage")) == NULL) {
        return GetLastError();
    }
    if ((PassThruReadVersion = (_PassThruReadVersion)GetProcAddress(hJ2534, "PassThruReadVersion")) == NULL) {
        return GetLastError();
    }
    if ((PassThruGetLastError = (_PassThruGetLastError)GetProcAddress(hJ2534, "PassThruGetLastError")) == NULL) {
        return GetLastError();
    }
    if ((PassThruIoctl = (_PassThruIoctl)GetProcAddress(hJ2534, "PassThruIoctl")) == NULL) {
        return GetLastError();
    }
    if ((PassThruOpen = (_PassThruOpen)GetProcAddress(hJ2534, "PassThruOpen")) == NULL) {
        return GetLastError();
    }
    if ((PassThruClose = (_PassThruClose)GetProcAddress(hJ2534, "PassThruClose")) == NULL) {
        return GetLastError();
    }
    if ((PassThruReset = (_PassThruReset)GetProcAddress(hJ2534, "PassThruReset")) == NULL) {
        return GetLastError();
    }
    if ((PassThruGetLastSocketError = (_PassThruGetLastSocketError)GetProcAddress(hJ2534, "PassThruGetLastSocketError")) == NULL) {
        return GetLastError();
    }
#else
    char* error = NULL;

    hJ2534 = dlopen("libJ2534.so", RTLD_LAZY);
    if (!hJ2534) {
        fprintf(stderr, "Error loading libJ2534.so %s\n", dlerror());
        return -1;
    }
    //clear existing errors
    dlerror();
    //map functions
    PassThruConnect = (_PassThruConnect)dlsym(hJ2534, "PassThruConnect");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruConnect %s\n", dlerror());
        return -1;
    }
    PassThruDisconnect = (_PassThruDisconnect)dlsym(hJ2534, "PassThruDisconnect");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruDisconnect %s\n", dlerror());
        return -1;
    }
    PassThruReadMsgs = (_PassThruReadMsgs)dlsym(hJ2534, "PassThruReadMsgs");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruReadMsgs %s\n", dlerror());
        return -1;
    }
    PassThruWriteMsgs = (_PassThruWriteMsgs)dlsym(hJ2534, "PassThruWriteMsgs");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruWriteMsgs %s\n", dlerror());
        return -1;
    }
    PassThruStartPeriodicMsg = (_PassThruStartPeriodicMsg)dlsym(hJ2534, "PassThruStartPeriodicMsg");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruStartPeriodicMsg %s\n", dlerror());
        return -1;
    }
    PassThruStopPeriodicMsg = (_PassThruStopPeriodicMsg)dlsym(hJ2534, "PassThruStopPeriodicMsg");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruStopPeriodicMsg %s\n", dlerror());
        return -1;
    }
    PassThruStartMsgFilter = (_PassThruStartMsgFilter)dlsym(hJ2534, "PassThruStartMsgFilter");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruStartMsgFilter %s\n", dlerror());
        return -1;
    }
    PassThruStopMsgFilter = (_PassThruStopMsgFilter)dlsym(hJ2534, "PassThruStopMsgFilter");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruStopMsgFilter %s\n", dlerror());
        return -1;
    }
    PassThruSetProgrammingVoltage = (_PassThruSetProgrammingVoltage)dlsym(hJ2534, "PassThruSetProgrammingVoltage");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruSetProgrammingVoltage %s\n", dlerror());
        return -1;
    }
    PassThruReadVersion = (_PassThruReadVersion)dlsym(hJ2534, "PassThruReadVersion");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruReadVersion %s\n", dlerror());
        return -1;
    }
    PassThruGetLastError = (_PassThruGetLastError)dlsym(hJ2534, "PassThruGetLastError");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruGetLastError %s\n", dlerror());
        return -1;
    }
    PassThruIoctl = (_PassThruIoctl)dlsym(hJ2534, "PassThruIoctl");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruIoctl %s\n", dlerror());
        return -1;
    }
    PassThruOpen = (_PassThruOpen)dlsym(hJ2534, "PassThruOpen");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruOpen %s\n", dlerror());
        return -1;
    }
    PassThruClose = (_PassThruClose)dlsym(hJ2534, "PassThruClose");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruClose %s\n", dlerror());
        return -1;
    }
    PassThruReset = (_PassThruReset)dlsym(hJ2534, "PassThruReset");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruReset %s\n", dlerror());
        return -1;
    }
    PassThruGetLastSocketError = (_PassThruGetLastSocketError)dlsym(hJ2534, "PassThruGetLastSocketError");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "J2534: PassThruGetLastSocketError %s\n", dlerror());
        return -1;
    }
#endif
    return 0;
}

bool unloadDLL() {
#ifdef _WIN32
    if (FreeLibrary(hJ2534))
#else
    if(dlclose(hJ2534) == 0)
#endif
    {
        PassThruConnect = NULL;
        PassThruDisconnect = NULL;
        PassThruReadMsgs = NULL;
        PassThruWriteMsgs = NULL;
        PassThruStartPeriodicMsg = NULL;
        PassThruStopPeriodicMsg = NULL;
        PassThruStartMsgFilter = NULL;
        PassThruStopMsgFilter = NULL;
        PassThruSetProgrammingVoltage = NULL;
        PassThruReadVersion = NULL;
        PassThruGetLastError = NULL;
        PassThruIoctl = NULL;
        PassThruOpen = NULL;
        PassThruClose = NULL;
        PassThruReset = NULL;
        PassThruGetLastSocketError = NULL;
        hJ2534 = NULL;
        return true;
    }

    return false;
}
