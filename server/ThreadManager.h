#pragma once

#include "Define.h"
#include "ThreadBase.h"

class ThreadManager
{
public:
	static	const uint32	WaitInfinite = INFINITE;
	static	const uint32	ForceTerminate = 0;
	static	const uint32	DefaultWaitTime = 2000;

public:
	ThreadManager(void);
	~ThreadManager(void);

	bool	Initialize(int32 maxThreadCount);
	void	Finalize();
	bool	StartThread(ThreadBase* pThread, id32* pId);
	bool	StopThread(id32 id, uint32 waitTime = WaitInfinite);
	bool	ResumeThread(id32 id);
	void	StopThreadAll();
	void	ResumeThreadAll();
	void	WaitForAllThreadTerminate(uint32 waitTime);
	ThreadBase*	GetThread(id32 id);

private:
	static	unsigned int __stdcall	Woker(void* args);
	int32	FindThread(id32 id);
	bool	StopThreadByIndex(int32 index, uint32 waitTime);
	bool	RemoveThrad(int32 index);

private:
	int32			_maxThreadCount;
	ThreadBase**	_pThreads;
	HANDLE*			_pThreadHandles;
	int32			_threadCount;
};