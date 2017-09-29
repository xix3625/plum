#pragma once

#include "Define.h"

class ThreadBase
{
public:
	static	const id32	InvalidId = 0LL;
	__declspec(thread)	static	ThreadBase*	Current;

public:
	ThreadBase();
	virtual ~ThreadBase();

	inline 	id32	GetId() { return _id; }
	inline	bool	IsRun() { return _isRun; }
	virtual	wchar*	Name() = 0;

protected:
	inline	void	SetRun(bool isRun) { _isRun = isRun; }
	inline	void	SetId(id32 id) { _id = id; }

	virtual	bool	Run() = 0;
	virtual	void	Stop() = 0;

private:
	id32	_id;
	bool	_isRun;

	friend class ThreadManager;
};