#pragma once

#include "Define.h"
#include "RefCountObject.h"
#include "IntrusivePtr.hpp"

#define INVALID_EVENTOBJECT_ID	0
#define INTERNAL_EVENTOBJECT_ID	1

enum {
	EVID_INVALID_ID = 0x00,
	EVID_IO_ACCEPT_ID,
	EVID_IO_CONNECT_ID,
	EVID_IO_RECV_ID,
	EVID_IO_SEND_ID,

	EVID_OTHER_ID,
};

enum {
	EVOBJ_ENDEVENT_ID = INTERNAL_EVENTOBJECT_ID,
	EVOBJ_LISTENER_ID,
	EVOBJ_SOCKET_ID,
	EVOBJ_TASK_ID,
	EVOBJ_ZONE_TASK_ID,

	EVOBJ_ID_MAX,
};

struct EventContext : public OVERLAPPED {
	uint32	id;
};

class IEventObject
{
public:
	virtual	uint32	EventId() const = 0;
	virtual	bool	HandleEvent(EventContext* pContext, ulong param) = 0;
};

class EventObject : public RefCountObject, IEventObject
{
public:
	virtual ~EventObject() {}

public:
	virtual	uint32	EventId() const = 0;
	virtual	bool	HandleEvent(EventContext* pContext, ulong param) = 0;
};

typedef IntrusivePtr<EventObject>	EventObjectPtr;