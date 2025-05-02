#pragma once
#include <Windows.h>

#define HTTP_REQUEST_FORMAT "%s %s HTTP/%d.%d\n\rHost: %*s\n\rUser-Agent %*s\n\rConnection: keep-alive\0"
#define MAX_PATH_LENGTH 4096

typedef enum HttpMethod { HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE } HttpMethod;
typedef HttpMethod* LPHttpMethod;

typedef struct HttpRequest
{
	HttpMethod method;
	LPTSTR path;
	WORD version;
}HttpRequest;
typedef HttpRequest* LPHttpRequest;

LPHttpRequest DecodeRequest(LPSTR request);