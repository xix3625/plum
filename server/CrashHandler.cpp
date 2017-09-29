#include "CrashHandler.h"
#include "Log.h"

CrashHandler::WriteMinidumpForExceptionFunc	CrashHandler::_pWriteMinidumpFunc = nullptr;

CrashHandler::CrashHandler()
{
}

CrashHandler::~CrashHandler()
{
}

bool CrashHandler::Initialize(WriteMinidumpForExceptionFunc pWriteMinidumpFunc)
{
	if (!pWriteMinidumpFunc)
		return false;

	_pWriteMinidumpFunc = pWriteMinidumpFunc;
	return true;
}

void CrashHandler::Finalize()
{
	_pWriteMinidumpFunc = nullptr;
}

long __stdcall CrashHandler::Handle(EXCEPTION_POINTERS *pExceptionInfo)
{
	if (_pWriteMinidumpFunc)
	{
		_pWriteMinidumpFunc(pExceptionInfo);
	}

	__try
	{
		LogSystem::Flush();
		Sleep(1000);
		exit(0);
	}
	__except (CrashHandler::HandleNoLog(pExceptionInfo)) {}

	return -1;
}

long __stdcall CrashHandler::HandleNoLog(EXCEPTION_POINTERS *pExceptionInfo)
{
	if (_pWriteMinidumpFunc)
	{
		_pWriteMinidumpFunc(pExceptionInfo);
	}

	return -1;
}