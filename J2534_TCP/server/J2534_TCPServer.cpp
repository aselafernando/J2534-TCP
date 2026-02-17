// J2534_Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include "getopt-win.h"
#pragma comment (lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#define INVALID_SOCKET -1
#define SOCKET int
#endif

#include <iostream>
#include <future>
#include <signal.h>

#include "J2534.h"
#include "J2534_Server.h"

#define SOCKET_TIMEOUT_SECONDS 5
#define LISTEN_PORT 2534

void on_client_connect(
    SOCKET client
);

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(
    DWORD dwCtrlType   //  control signal type
);
#endif
static bool stopServer = false;
bool verbose = false;
static bool memoryLeakMode = false;

using namespace std;

int main(int argc, char* argv[])
{
    sockaddr_in server_addr, client_addr;
    int result;
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
    #endif
    const auto server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    const char* listenAddr = NULL;
    int listenPort = LISTEN_PORT;
    int c;
    #ifdef _WIN32
        DWORD timeout = SOCKET_TIMEOUT_SECONDS * 1000;
        setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    #else //_WIN32
        struct timeval timeout;
        timeout.tv_sec = SOCKET_TIMEOUT_SECONDS;
        timeout.tv_usec = 0;
        setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    #endif

    /* Disabling output buffers */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    while ((c = getopt(argc, argv, "l:p:mv")) != -1)
    {
        switch (c)
        {
            case 'l':
                listenAddr = optarg;
                break;
            case 'p':
                if (optarg != NULL) {
                    listenPort = atoi(optarg);
                }
                break;
            case 'v':
                verbose = true;
                fprintf(stdout, "Verbose active\n");
                break;
            case 'm':
                memoryLeakMode = true;
                fprintf(stdout, "Memory Leak Mode active\n");
                //Single threaded, exit after disconnect to free memory
                //Useful if DLL has memory leaks ie MVCI32.dll
                break;
        }
    }

    /*if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE) {
        // unable to install handler... 
        // display message to the user
        fprintf(stderr, "Unable to install handler!\n");
        return -1;
    }*/

    fprintf(stdout, "Loading J2534 library\n");

    if (result = loadDLL()) {
        fprintf(stderr, "Error: %d\n", result);
        return result;
    }

    fprintf(stdout, "Loaded J2534 library\n");
    
    if (listenAddr) {
        #ifdef _WIN32
            InetPtonA(AF_INET, listenAddr, &server_addr.sin_addr.s_addr);
        #else
            server_addr.sin_addr.s_addr = inet_addr(listenAddr);
        #endif
    } else {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listenPort);

    result = ::bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    if (result != 0) {
        if (listenAddr) {
            #ifdef _WIN32
                fprintf(stderr, "Unable to bind to port %s:%d! Error: %d\n", listenAddr, listenPort, WSAGetLastError());
            #else
                fprintf(stderr, "Unable to bind to port %s:%d! Error: %d\n", listenAddr, listenPort, result);
            #endif
        }
        else {
            #ifdef _WIN32
                fprintf(stderr, "Unable to bind to port %d! Error: %d\n", listenPort, WSAGetLastError());
            #else
                fprintf(stderr, "Unable to bind to port %d! Error: %d\n", listenPort, result);
            #endif
        }
        return result;
    }

    result = listen(server, 0);
    if (result != 0) {
        if (listenAddr) {
            #ifdef _WIN32
                fprintf(stderr, "Unable to listen to port %s:%d! Error: %d\n", listenAddr, listenPort, WSAGetLastError());
            #else
                fprintf(stderr, "Unable to listen to port %s:%d! Error: %d\n", listenAddr, listenPort, result);
            #endif
        }
        else {
            #ifdef _WIN32
                fprintf(stderr, "Unable to listen to port %d! Error: %d\n", listenPort, WSAGetLastError());
            #else
                fprintf(stderr, "Unable to listen to port %d! Error: %d\n", listenPort, result);
            #endif
        }
        return result;
    }

    if (listenAddr) {
        fprintf(stdout, "Listening for incoming connections on %s:%d...\n", listenAddr, listenPort);
    }
    else {
        fprintf(stdout, "Listening for incoming connections on port %d...\n", listenPort);
    }

    #ifdef _WIN32
        int32_t client_addr_size = sizeof(client_addr);
    #else
        uint32_t client_addr_size = sizeof(client_addr);
    #endif

    while (stopServer == false)
    {
        SOCKET client;
        if ((client = accept(server, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_size)) != INVALID_SOCKET)
        {
            if (memoryLeakMode) {
                on_client_connect(client);
                stopServer = true; //terminate after client because of memory leaks in the DLL
            } else {
                auto fut = async(launch::async, on_client_connect, client);
            }
        }
        #ifdef _WIN32
            const auto last_error = WSAGetLastError();

            if (last_error > 0)
            {
                fprintf(stderr, "WSA Error: %d\n", last_error);
            }
        #endif
    }

    #ifdef _WIN32
        closesocket(server);
    #else
        close(server);
    #endif

    fprintf(stdout, "Server stopped listening\n");

    fprintf(stdout, "Unloading J2534 library\n");
    unloadDLL();

    fprintf(stdout, "Exiting..\n");
    return 0;
}

void on_client_connect(SOCKET client)
{
    uint32_t buffLen = 0;
    int32_t readLen,sentLen = 0;
    uint32_t i = 0;
    CommandPacket* cmd = NULL;
    ReplyPacket* reply = NULL;

    fprintf(stdout, "Client connected!\n");

    while (stopServer == false) {
        buffLen = 0;
        readLen = 0;
        sentLen = 0;

        if (cmd) free(cmd);
        if (reply) free(reply);
        cmd = NULL;
        reply = NULL;

        for (i = 0; i < sizeof(buffLen); i += readLen) {
            readLen = recv(client, (char*)&buffLen, sizeof(buffLen) - i, 0);
            if(verbose)
                fprintf(stdout, "Read %d bytes\n", readLen);

            if (readLen <= 0) {
                #ifdef _WIN32
                    fprintf(stderr, "WSA Error: %d\n", WSAGetLastError());
                #endif
                goto disconnect;
            }
        }

        if (verbose)
            fprintf(stdout, "Command Length = %d bytes \n", buffLen);

        if ((cmd = (CommandPacket*)malloc(sizeof(uint8_t) * buffLen)) != NULL) {
            cmd->len = buffLen;
            buffLen -= sizeof(buffLen); //we already read bytes for the length
            for (i = 0; i < buffLen; i += readLen) {
                readLen = recv(client, (char*)cmd + sizeof(buffLen) + i, buffLen - i, 0);
                if (verbose)
                    fprintf(stdout, "Read %d bytes\n", readLen);
                if (readLen <= 0) {
                    #ifdef _WIN32
                        fprintf(stderr, "WSA Error: %d\n", WSAGetLastError());
                    #endif
                    goto disconnect;
                }
            }

            reply = processCommand(cmd);

            if (reply != NULL) {
                if (verbose)
                    fprintf(stdout, "reply result = %d\n", reply->result);

                for (i = 0; i < reply->len; i += sentLen) {
                    sentLen = send(client, (char*)reply + i, reply->len - i, 0);
                    if (verbose)
                        fprintf(stdout, "sentLen %d\n", sentLen);
                    if (sentLen <= 0) {
                        #ifdef _WIN32
                            fprintf(stderr, "WSA Error: %d\n", WSAGetLastError());
                        #endif
                        goto disconnect;
                    }
                }
                if (verbose)
                    fprintf(stdout, "sent reply\n");

                /*if ((reply->cmd == J2534_Command::OPEN &&
                    reply->result != RETURN_STATUS::STATUS_NOERROR) ||
                    (reply->cmd == J2534_Command::CLOSE &&
                        reply->result == RETURN_STATUS::STATUS_NOERROR)) {
                    cout << "Disconnecting.." << endl;
                    free(reply);
                    free(cmd);
                    goto disconnect;
                }*/
            }
        }
    }

disconnect:
    if (cmd) free(cmd);
    if (reply) free(reply);
    #ifdef _WIN32
        closesocket(client);
    #else
        close(client);
    #endif
    fprintf(stdout, "Client disconnected.\n");
    return;
}
#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD CEvent) {

    switch (CEvent) {
        case CTRL_C_EVENT:
            stopServer = true;
            break;
        case CTRL_BREAK_EVENT:
            stopServer = true;
            break;
        case CTRL_CLOSE_EVENT:
            stopServer = true;
            break;
        case CTRL_LOGOFF_EVENT:
            stopServer = true;
            break;
        case CTRL_SHUTDOWN_EVENT:
            stopServer = true;
            break;
    }

    return TRUE;
}
#endif
