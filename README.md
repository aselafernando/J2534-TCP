# J2534-TCP

This project builds a client and a server in order to share a J2534 library over a TCP/IP connection.
The primary reason I created this was toto share a proprietary J2534 DLL with a Linux program using Wine.

---

## Project Overview

* Cross-platform: Server and client both support Windows and Linux builds
* Architecture support: Fully supports both `i386` and `x86_64`, targets

## Requirements
 - Windows: Visual Studio 
 - Linux: g++

## Build Instructions

### Linux

To build the client and server

```
make
```

To clean all generated files:

```
make clean
```

### Windows

Open `J2534_TCP.sln` in Visual Studio. The targets available are

`J2534_TCPClient Debug/Release x86/x64`

`J2534_TCPServer Debug/Release x86/x64`

## Options

### Client

The client build output is a library (libJ2534.so for Linux or J2534.dll for Windows) and is designed to connect to the server in this repository.
It is currently hardcoded to seek a server on 127.0.0.1:2534. This is defined in `client/dllmain.cpp`

### Server

`-m` Enable a single threaded "memory leak" mode. This is because some J2534 libraries have memory issues. In order to manage this the program will terminate upon the client closing the connection to release the memory back.

`-l 127.0.0.1` The listening address for the server

`-p 2534` The listening port

`-v` Enable verbose mode to show functions called and bytes transferred

## Communication

All communication over TCP/IP is transparent to the client application. In the eyes of the client it's exactly the same as if the vehicle interface cable is connected directly, but thanks to TCP/IP it may be another computer somewhere on a network.

### Client
The desired program will load the client library (libJ2534.so in Linux or J2534.dll in Windows). The client library should be renamed to whatever the program expects it to be.

Upon the program calling `PassThruOpen()` a TCP connection will be initiated to the server on 127.0.0.1:2534

Function calls are serialised and sent to the server, the results are returned and sent back to the application.

### Server

The server loads a proprietary libj2534.so (Linux) or J2534.dll (Windows) which interfaces with a vehicle interface cable.

It then listens on the specified port (default 127.0.0.1:2534) for the client to connect.

The server will take a serialized command from the client, execute it against the J2534 library and then send the result back.

## License

This project is licensed under the GNU General Public License v2 (GPLv2).
