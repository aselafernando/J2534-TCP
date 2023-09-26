// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma pack(1)
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#ifdef _WIN32
#define WINDOWS_IGNORE_PACKING_MISMATCH
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "targetver.h"
#include <Windows.h>
#include <tchar.h>
#include <malloc.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1
#define SOCKET int
#endif

#include <iostream>
#include <fstream>

using namespace std;

#include "J2534_V0404.h"
#include "J2534_TCPIP.h"
#include "J2534_Client.h"
#include "ini.h"

extern std::ofstream outfile;
extern bool doLog;
extern std::string serverAddr;
extern int serverPort;

extern void readINI();