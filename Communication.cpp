#include "Communication.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Communication::~Communication() {
	Disconnect();
}

bool Communication::Connect() {
	HRESULT hr = FilterConnectCommunicationPort(
		FILEAWARE_PORT_NAME,
		0,
		nullptr,
		0,
		nullptr,
		&hPort
	);

	if (FAILED(hr)) {
		std::wcout << L"Failed to connect to filter port: 0x" << std::hex << hr << std::endl;
		return false;
	}

	hCompletion = CreateIoCompletionPort(hPort, nullptr, 0, 0);
	if (hCompletion == nullptr) {
		std::wcout << L"Failed to create completion port: " << GetLastError() << std::endl;
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
		return false;
	}

	std::wcout << L"Connected to file monitor driver successfully" << std::endl;
	return true;
}

void Communication::Disconnect() {
	if (hCompletion != INVALID_HANDLE_VALUE) {
		CloseHandle(hCompletion);
		hCompletion = INVALID_HANDLE_VALUE;
	}

	if (hPort != INVALID_HANDLE_VALUE) {
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
	}
}

std::wstring Communication::FormatTime(const LARGE_INTEGER& timeStamp) {
	FILETIME ft;
	SYSTEMTIME st;
	std::wstringstream ss;

	ft.dwLowDateTime = timeStamp.LowPart;
	ft.dwHighDateTime = timeStamp.HighPart;

	FileTimeToSystemTime(&ft, &st);

	ss << std::setw(2) << std::setfill(L'0') << st.wHour << L":"
		<< std::setw(2) << std::setfill(L'0') << st.wMinute << L":"
		<< std::setw(2) << std::setfill(L'0') << st.wSecond << L"."
		<< std::setw(3) << std::setfill(L'0') << st.wMilliseconds;

	return ss.str();
}

void Communication::Monitor() {
	if (hPort == INVALID_HANDLE_VALUE) {
		std::wcout << L"Not connected to driver" << std::endl;
		return;
	}

	std::wcout << L"Starting file monitor..." << std::endl;
	std::wcout << L"Press Ctrl+C to stop monitoring" << std::endl << std::endl;

	while (true) {
		FILEAWARE_MESSAGE message = { 0 };
		FILEAWARE_REPLY reply = { 0 };
		DWORD bytesReturned = 0;
		OVERLAPPED overlapped = { 0 };
		ULONG_PTR key = 0;
		LPOVERLAPPED lpOverlapped = nullptr;

		HRESULT hr = FilterGetMessage(hPort,
			reinterpret_cast<PFILTER_MESSAGE_HEADER>(&message),
			sizeof(message),
			&overlapped);

		if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
			DWORD waitResult = WaitForSingleObject(hCompletion, INFINITE);
			if (waitResult == WAIT_OBJECT_0) {
				if (GetQueuedCompletionStatus(hCompletion,
					&bytesReturned,
					&key,
					&lpOverlapped,
					INFINITE)) {

					if (bytesReturned >= sizeof(FILEAWARE_MESSAGE)) {
						DisplayFileEvent(&message);
					}
				}
			}
		}
		else if (SUCCEEDED(hr)) {
			DisplayFileEvent(&message);
		}
		else {
			std::wcout << L"FilterGetMessage failed: 0x" << std::hex << hr << std::endl;
			break;
		}

		// ·¢ËÍ»Ø¸´
		reply.Status = 0x12345678;
		FilterReplyMessage(hPort,
			reinterpret_cast<PFILTER_REPLY_HEADER>(&reply),
			sizeof(reply));
	}
}

void Communication::DisplayFileEvent(const PFILEAWARE_MESSAGE message) {
	std::wcout << L"[" << FormatTime(message->TimeStamp) << L"] "
		<< L"Process: " << message->ProcessId
		<< L" | Thread: " << message->ThreadId
		<< L" | Type: " << message->FileType
		<< L" | File: " << message->FileName
		<< L" | Path: " << message->FilePath
		<< std::endl;
}