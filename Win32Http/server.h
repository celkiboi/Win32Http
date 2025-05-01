#pragma once
#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include "errors.h"

#define BUFFER_SIZE 4096

INT32 StartServer(LPTSTR ipAddress, UINT16 port, UINT8 threadCount);