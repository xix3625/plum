#pragma once

#include "Define.h"
#include "ThreadBase.h"
#include "INetwork.h"

class EventObject;
struct EventContext;

class IOThread : public ThreadBase
{
public:
	__declspec(thread)	static	id32	IOThreadId;

public:
	IOThread(void);
	virtual ~IOThread(void);
	static	id32	GetIOThreadId() { return IOThreadId; }

	bool	Initialize(id32 id, INetwork* pNetwork, FailedHandleEventFunc pFailedHandleEventFunc);
	wchar*	Name() override { return L"IOThread"; }

private:
	bool	Run() override;
	void	Stop() override;

	void	FailedHandleEvent(EventObject* pEventObject, EventContext* pEventContext, ulong param);

private:
	INetwork*				_pNetwork;
	id32					_ioThreadId;
	FailedHandleEventFunc	_pFailedHandleEventFunc;
};