#pragma once

#include "Define.h"

class CriticalSection
{
public:
	CriticalSection() {
		InitializeCriticalSection(&_cs);
	}
	~CriticalSection() {
		DeleteCriticalSection(&_cs);
	}

	inline void	Enter()		{ EnterCriticalSection(&_cs); }
	inline void	Leave()		{ LeaveCriticalSection(&_cs); }
	inline bool	TryEnter()	{ return (TRUE == TryEnterCriticalSection(&_cs)) ? true : false; }

private:
	CRITICAL_SECTION	_cs;
};

template <class _LockType>
class TAutoLock
{
	TAutoLock() {}

public:
	TAutoLock(const _LockType &lock)
	{
		_lock = (_LockType*)&lock;
		_lock->Enter();
	}

	~TAutoLock()
	{
		if (_lock)
			_lock->Leave();
	}

	void Free()
	{
		if (_lock)
			_lock->Leave();
		_lock = nullptr;
	}

private:
	_LockType*	_lock;

	// ---------------------------------------------------------------------------
	//! No copies do not implement
	TAutoLock(const TAutoLock &);
	TAutoLock& operator =(const TAutoLock &);
};