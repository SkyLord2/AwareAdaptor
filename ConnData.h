#pragma once
#include <Windows.h>
#include <fltUser.h>
#pragma comment(lib, "fltLib.lib")

#define FILEAWARE_PORT_NAME L"\\FileMonitorPort"

typedef struct _FILEAWARE_MESSAGE {
	ULONG Type;
	ULONG ProcessId;
	ULONG ThreadId;
	WCHAR FileName[256];
	WCHAR FilePath[512];
	WCHAR FileType[64];
	LARGE_INTEGER TimeStamp;
} FILEAWARE_MESSAGE, * PFILEAWARE_MESSAGE;

typedef struct _FILEMONITOR_REPLY {
	ULONG Status;
} FILEAWARE_REPLY, * PFILEAWARE_REPLY;
