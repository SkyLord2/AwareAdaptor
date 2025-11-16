#pragma once
#include <Windows.h>
#include <string>
#include "ConnData.h"

class Communication
{
private:
	HANDLE hPort;
	HANDLE hCompletion;
public:
    Communication() : hPort(INVALID_HANDLE_VALUE), hCompletion(INVALID_HANDLE_VALUE) {}
	~Communication();
	bool Connect();
	void Disconnect();
	std::wstring FormatTime(const LARGE_INTEGER& timeStamp);
	void Monitor();
	void DisplayFileEvent(const PFILEAWARE_MESSAGE message);
};