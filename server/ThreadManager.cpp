#include "ThreadManager.h"
#include <process.h>
#include "CrashHandler.h"

ThreadManager::ThreadManager(void)
	: _maxThreadCount(0), _pThreads(nullptr), _pThreadHandles(nullptr), _threadCount(0)
{
}

ThreadManager::~ThreadManager(void)
{
}

bool ThreadManager::Initialize(int32 maxThreadCount)
{
	if (0 >= maxThreadCount) {
		return false;
	}

	_maxThreadCount = maxThreadCount;
	_pThreads = alloc_d(ThreadBase*, _maxThreadCount);
	_pThreadHandles = alloc_d(HANDLE, _maxThreadCount);
	for (int32 index = 0; index < _maxThreadCount; ++index)
	{
		_pThreads[index] = nullptr;
		_pThreadHandles[index] = INVALID_HANDLE_VALUE;
	}

	return true;
}

void ThreadManager::Finalize()
{
	StopThreadAll();

	if (_pThreads) {
		free_d(_pThreads);
		_pThreads = nullptr;
	}

	if (_pThreadHandles) {
		free_d(_pThreadHandles);
		_pThreadHandles = nullptr;
	}
}

bool ThreadManager::StartThread(ThreadBase* pThread, id32* pId)
{
	if (_maxThreadCount <= _threadCount) {
		return false;
	}

	int32 index = -1;
	for (index = 0; index < _maxThreadCount; ++index) {
		if (nullptr == _pThreads[index])
			break;
	}
	if (0 > index || _maxThreadCount <= index) {
		return false;
	}

	uint32 threadId = 0;
	_pThreadHandles[index] = (HANDLE)_beginthreadex(NULL, 0, ThreadManager::Woker, pThread, CREATE_SUSPENDED, &threadId);
	if (!_pThreadHandles[index]) {
		_pThreads[index] = nullptr;
		_pThreadHandles[index] = INVALID_HANDLE_VALUE;
		return false;
	}

	_pThreads[index] = pThread;
	if (pId)
		*pId = threadId;

	InterlockedIncrement((volatile uint32*)&_threadCount);

	return true;
}

bool ThreadManager::StopThread(id32 id, uint32 waitTime /* = WaitInfinite */)
{
	int32 index = FindThread(id);
	if (0 < index || _maxThreadCount <= index) {
		return false;
	}

	return StopThreadByIndex(index, waitTime);
}

bool ThreadManager::ResumeThread(id32 id)
{
	uint32 index = FindThread(id);
	if (index < 0 || _pThreads[index]) {
		return false;
	}

	::ResumeThread(_pThreadHandles[index]);

	return true;
}

void ThreadManager::StopThreadAll()
{
	for (int32 index = 0; index < _maxThreadCount; ++index)
	{
		if (_pThreads[index])
			StopThread(index, ForceTerminate);
	}
}

void ThreadManager::ResumeThreadAll()
{
	for (int32 index = 0; index < _maxThreadCount; ++index)
	{
		if (_pThreads[index])
			::ResumeThread(_pThreadHandles[index]);
	}
}

void ThreadManager::WaitForAllThreadTerminate(uint32 waitTime)
{
	for (int32 index = 0; index < _maxThreadCount; ++index)
	{
		if (_pThreads[index])
			_pThreads[index]->Stop();
	}

	for (int32 index = 0; index < _maxThreadCount; ++index)
	{
		if (_pThreads[index])
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(_pThreadHandles[index], waitTime)) {
				TerminateThread(_pThreadHandles[index], 0);
			}

			CloseHandle(_pThreadHandles[index]);
			RemoveThrad(index);
		}
	}
}

ThreadBase*	ThreadManager::GetThread(id32 id)
{
	int32 index = FindThread(id);
	if (0 < index || _maxThreadCount <= index) {
		return nullptr;
	}

	return _pThreads[index];
}

unsigned int __stdcall	ThreadManager::Woker(void* args)
{
	if (!args)
		return 0;

	__try {
		ThreadBase* pThread = reinterpret_cast<ThreadBase*>(args);
		pThread->Current = pThread;
		pThread->SetId(GetCurrentThreadId());
		pThread->SetRun(true);
		pThread->Run();
		pThread->SetRun(false);
	}
	__except (CrashHandler::Handle(GetExceptionInformation())) {}

	return 0;
}
int32 ThreadManager::FindThread(id32 id)
{
	for (int32 index = 0; index < _maxThreadCount; ++index)
	{
		if (_pThreads[index] && _pThreads[index]->GetId() == id)
			return index;
	}

	return -1;
}

bool ThreadManager::StopThreadByIndex(int32 index, uint32 waitTime)
{
	if (0 > index || _maxThreadCount <= index) {
		return false;
	}

	if (!_pThreads[index]) {
		return false;
	}

	bool force = (ForceTerminate == waitTime) ? true : false;
	if (force) {
		waitTime = DefaultWaitTime;
	}

	_pThreads[index]->Stop();
	if (WAIT_OBJECT_0 != WaitForSingleObject(_pThreadHandles[index], waitTime))
	{
		if (!force)
			return false;

		TerminateThread(_pThreadHandles[index], 0);
	}
	CloseHandle(_pThreadHandles[index]);

	return RemoveThrad(index);
}

bool ThreadManager::RemoveThrad(int32 index)
{
	if (0 > index || _maxThreadCount <= index) {
		return false;
	}

	if (!_pThreads[index]) {
		return false;
	}	

	delete _pThreads[index];
	_pThreads[index] = nullptr;
	_pThreadHandles[index] = INVALID_HANDLE_VALUE;
	InterlockedDecrement((volatile uint32*)&_threadCount);

	return true;
}