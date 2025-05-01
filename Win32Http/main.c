#include <stdio.h>

#include "errors.h"
#include "server.h"

INT32 _tmain(INT32 argc, LPTSTR argv[])
{
	if (argc != 4)
	{
		_tprintf(_T("Error. Incorrect number of params.\nUsage: %s [IP] [PORT] [THREAD COUNT]"), argv[0]);
		return  INCORRECT_PARAM_NUMBERS;
	}

	UINT16 port = _ttoi(argv[2]);
	LPTSTR ipAddress = argv[1];
	UINT8 threadCount = _ttoi(argv[3]);

	INT32 exitCode = StartServer(ipAddress, port, threadCount);

	return exitCode;
}