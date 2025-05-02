#include "http.h"
#include <stdio.h>

/*
	Network communication always results in ASCII characters
	To handle the file system on windows, we need to support both ASCII and UNICODE
	This is done trough this entire project using TCHAR, so we need to convert ASCII
	to whatever TCHAR is currently representing
*/
LPTSTR ConvertToTSTR(LPSTR input, SIZE_T size)
{
	LPTSTR output = NULL;
#ifdef UNICODE
	SIZE_T requiredSize = MultiByteToWideChar(CP_ACP, 0, input, -1, NULL, 0);
	if (requiredSize == 0) return NULL;

	output = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, requiredSize * sizeof(WCHAR));
	if (output == NULL) return NULL;

	MultiByteToWideChar(CP_ACP, 0, input, -1, output, requiredSize);
#else
	output = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, size);
	if (output == NULL) return NULL;

	lstrcpynA(output, input, (int)size);
#endif
	return output;
}

LPHttpRequest DecodeRequest(LPSTR request)
{
	CHAR method[10] = { '\0' };
	INT32 majorVersion;
	INT32 minorVersion;
	CHAR asciiPath[MAX_PATH_LENGTH] = { '\0' };

	sscanf_s(request, HTTP_REQUEST_FORMAT, method, 10, asciiPath, MAX_PATH_LENGTH - 1, &majorVersion, &minorVersion);

	HttpMethod httpMethod = 0;
	if (lstrcmpiA(method, "GET") == 0)
		httpMethod = HTTP_GET;
	else if (lstrcmpiA(method, "PUT") == 0)
		httpMethod = HTTP_PUT;
	else if (lstrcmpiA(method, "POST") == 0)
		httpMethod = HTTP_POST;
	else if (lstrcmpiA(method, "PATCH") == 0)
		httpMethod = HTTP_PATCH;
	else if (lstrcmpiA(method, "DELETE") == 0)
		httpMethod = HTTP_DELETE;

	LPHttpRequest httpRequest = (LPHttpRequest)HeapAlloc(GetProcessHeap(), 0, sizeof(HttpRequest));
	if (httpRequest == NULL)
		return NULL;

	LPTSTR path = ConvertToTSTR(&asciiPath, lstrlenA(&asciiPath));

	httpRequest->version = MAKEWORD(majorVersion, minorVersion);
	httpRequest->path = path;
	httpRequest->method = httpMethod;

	return httpRequest;
}