#include "ThreadBase.h"

__declspec(thread)	ThreadBase*	ThreadBase::Current = nullptr;

ThreadBase::ThreadBase()
	: _id(InvalidId), _isRun(false)
{
}

ThreadBase::~ThreadBase()
{
}