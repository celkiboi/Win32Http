#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include "errors.h"

#define BUFFER_SIZE 4096

typedef struct ThreadParam 
{
	UINT8 threadNumber;
	SOCKET clientSocket;
}THREAD_PARAMS;

CHAR response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\nConnection: close\r\n\r\nHello world!";

DWORD WINAPI HandleClientConnection(LPVOID lpParam)
{
	SOCKET clientSocket = ((THREAD_PARAMS*)lpParam)->clientSocket;
	UINT8 threadNumber = ((THREAD_PARAMS*)lpParam)->threadNumber;

	_tprintf(_T("Thread number: %d responding to a request\n"), threadNumber);
	
	printf("%s\n", response);
	send(clientSocket, response, lstrlenA(response), 0);

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);

	ExitThread(0);
}

INT32 _tmain(INT32 argc, LPTSTR argv[])
{
	INT32 exitCode = 0;
	SOCKET* clientSockets = NULL;
	SOCKET listenSocket = INVALID_SOCKET;
	THREAD_PARAMS* threadParams = NULL;

	if (argc != 4)
	{
		_tprintf(_T("Error. Incorrect number of params.\nUsage: %s [IP] [PORT] [THREAD COUNT]"), argv[0]);
		exitCode = INCORRECT_PARAM_NUMBERS;
		goto exit;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		_tprintf(_T("Cannot start Winsock. Error: %d"), WSAGetLastError());
		exitCode = CANNOT_START_WSA;
		goto exit;
	}

	UINT16 port = _ttoi(argv[2]);
	LPTSTR ipAddress = argv[1];
	UINT8 threadCount = _ttoi(argv[3]);

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listenSocket == INVALID_SOCKET)
	{
		_tprintf(_T("Cannot create a listening socket. Error: %d"), WSAGetLastError());
		exitCode = CANNOT_CREATE_SOCKET;
		goto exit;
	}

	SOCKADDR_IN listenSocketAddr = { 0 };
	listenSocketAddr.sin_family = AF_INET;
	listenSocketAddr.sin_port = htons(port);
	if (InetPton(AF_INET, ipAddress, &listenSocketAddr.sin_addr) != 1)
	{
		_tprintf(_T("Invalid IP adress format. Error :%d"), WSAGetLastError());
		exitCode = INVALID_IP_ADRESS;
		goto exit;
	}

	if (bind(listenSocket, (SOCKADDR*)&listenSocketAddr, sizeof(listenSocketAddr)) == SOCKET_ERROR)
	{
		_tprintf(_T("Listen socket bind failed. Error: %d"), WSAGetLastError());
		exitCode = SOCKET_BIND_FAIL;
		goto exit;
	}

	
	clientSockets = (SOCKET*)HeapAlloc(GetProcessHeap(), 0, threadCount * sizeof(SOCKET));
	if (clientSockets == NULL)
	{
		_tprintf(_T("Cannot allocate memory for %d client sockets."), threadCount);
		exitCode = CANNOT_ALLOCATE_CLIENT_SOCKETS;
		goto exit;
	}

	for (UINT8 i = 0; i < threadCount; i++)
	{
		clientSockets[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (clientSockets[i] == INVALID_SOCKET)
		{
			_tprintf(_T("Cannot create a client socket number %d. Error: %d"), i, WSAGetLastError());
			exitCode = CANNOT_CREATE_SOCKET;
			goto exit;
		}
	}

	HANDLE* clientThreadHandles = NULL;
	clientThreadHandles = (HANDLE*)HeapAlloc(GetProcessHeap(), 0, threadCount * sizeof(HANDLE));
	if (clientThreadHandles == NULL)
	{
		_tprintf(_T("Cannot allocate thread handles."));
		exitCode = CANNOT_ALLOCATE_THREADS;
		goto exit;
	}

	threadParams = (THREAD_PARAMS*)HeapAlloc(GetProcessHeap(), 0, threadCount * sizeof(THREAD_PARAMS));
	if (threadParams == NULL)
	{
		_tprintf(_T("Cannot allocate memory for thread params."));
		exitCode = CANNOT_ALLOCATE_THREADS;
		goto exit;
	}

	if (listen(listenSocket, threadCount) == SOCKET_ERROR)
	{
		_tprintf(_T("Error listening on socket. Error: %d"), WSAGetLastError());
		exitCode = CANNOT_LISTEN;
		goto exit;
	}

	for (UINT8 i = 0; i < threadCount;)
	{
		clientSockets[i] = accept(listenSocket, NULL, NULL);
		if (clientSockets[i] == INVALID_SOCKET)
		{
			_tprintf(_T("Cannot accept a client socket. Error: %d"), WSAGetLastError());
			continue;

		}
		threadParams[i].threadNumber = i;
		threadParams[i].clientSocket = clientSockets[i];

		clientThreadHandles[i] = CreateThread(NULL, 0, HandleClientConnection, (LPVOID)&threadParams[i], 0, NULL);

		i++;
	}

	WaitForMultipleObjects(threadCount, clientThreadHandles, TRUE, INFINITE);

exit:
	if (clientSockets != NULL)
	{
		for (UINT8 i = 0; i < threadCount; i++)
		{
			if (clientSockets[i] != INVALID_SOCKET)
				closesocket(clientSockets[i]);
		}
		HeapFree(GetProcessHeap(), 0, clientSockets);
	}

	if (listenSocket != INVALID_SOCKET)
		closesocket(listenSocket);

	if (threadParams == NULL)
		HeapFree(GetProcessHeap(), 0, threadParams);

	WSACleanup();
	return exitCode;
}