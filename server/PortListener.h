#pragma once

#include "Define.h"
#include "EventObject.h"

class Port;
class Socket;

class PortListener : public EventObject
{
public:
	enum {
		CONTEXT_COUNT = 128,
		CONTEXT_BUFFER = 128,
	};

	struct AcceptContext : EventContext
	{
		uint32	index;
		SOCKET	hSocket;
		char	buffer[CONTEXT_BUFFER];
		ulong	recvedSize;
		ulong	localAddr;
		ulong	remoteAddr;
	};

public:
	PortListener(void);
	virtual ~PortListener(void);

	bool	Initialize(Port* pPort, ulong ip, ushort portNo, bool nagle);
	void	Finalize();

	uint32	EventId() const override { return EVOBJ_LISTENER_ID; }
	bool	HandleEvent(EventContext* pContext, ulong param) override;
	void	Delete() override
	{
		delete this;
	}

private:
	bool	AsyncAccept(uint32 index);
	bool	Accept(AcceptContext* pContext);

private:
	AcceptContext	_context[CONTEXT_COUNT];
	SOCKET			_hSocket;
	Port*			_pPort;
	ulong			_ip;
	ushort			_portNo;

	//TODO ref Release 처리로 추가 - 다른 방법을 찾자 by riverstyx
	friend Port;
};