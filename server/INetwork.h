#pragma once

#include "Define.h"

class Port;
class EventObject;
class TaskManager;

class INetwork
{
public:
	INetwork(void) {}
	virtual ~INetwork(void) {}
	virtual	Port*			GetPort(uint32 portId) = 0;
	virtual	EventObject*	EndEvent() = 0;
	virtual	HANDLE			IocpHandle() = 0;
	virtual	bool			IsRun() = 0;
	virtual	int				GetThreadCount() = 0;
};

typedef	void(*FailedHandleEventFunc)(INetwork*, EventObject*);
