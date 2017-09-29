#pragma once

#include "Define.h"

class CrashHandler
{
public:
	typedef	void(*WriteMinidumpForExceptionFunc)(EXCEPTION_POINTERS* pExinfo);

public:
	~CrashHandler();
	static	bool	Initialize(WriteMinidumpForExceptionFunc pWriteMinidumpFunc);
	static	void	Finalize();
	static	long	__stdcall	Handle(EXCEPTION_POINTERS* pExceptionInfo);
	static	long	__stdcall	HandleNoLog(EXCEPTION_POINTERS* pExceptionInfo);

protected:
	CrashHandler();

private:
	static	WriteMinidumpForExceptionFunc	_pWriteMinidumpFunc;
};