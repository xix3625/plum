#pragma once

#include "Define.h"
#include <mswsock.h>
#include "INetwork.h"
#include "PacketBufferHandler.h"

class INetworkNotify;
class EventObject;
class NetworkEndEvent;
class ThreadManager;
class Port;
class Session;
struct NetworkDesc;

class Network : public INetwork
{
public:
	static	const uint32	InvalidPortId = 0;
	enum {
		MAX_PORT_COUNT = 8,
	};

public:
	Network(ThreadManager* pThreadManager);
	virtual ~Network(void);

	bool	Initialize(int32 threadCount, FailedHandleEventFunc pFailedaHandleEvent);
	void	Finalize();

	int		NewPort(const NetworkDesc& desc, INetworkNotify* pNotify, IPacketBufferHandler* pHandler = nullptr);
	bool	Listen(uint32 portId, const wchar* pszIP, const ushort portNo);
	bool	Connect(uint32 portId, const wchar* pszIP, const ushort portNo, uint32 connectTimeout, bool async = true);
	bool	ConnectSession(uint32 portId, const wchar* pszIP, const ushort portNo, Session* pSession, uint32 connectTimeout, bool async = true);

	Port*			GetPort(uint32 portId) override;
	EventObject*	EndEvent() override	{ return (EventObject*)_pEndEvent; }
	HANDLE			IocpHandle() override { return _iocp; }
	bool			IsRun() override { return _run; }
	int				GetThreadCount() override { return _threadCount; }
	inline uint32	GetPortIndex(uint32 portId) { return (portId - 1); }

	static	inline	LPFN_ACCEPTEX				GetAcceptExFunc() { return lpfnAcceptEx; }
	static	inline	LPFN_GETACCEPTEXSOCKADDRS	GetAcceptExSockAddrsFunc() { return lpfnGetAcceptExSockAddrs; }
	static	inline	LPFN_CONNECTEX				GetConnectExFunc() { return lpfnConnectEx; }
	static	inline	LPFN_DISCONNECTEX			GetDisconnectExFunc() { return lpfnDisconnectEx; }

	static	bool	Startup();
	static	void	Cleanup();
	static	ulong	GetIpAddress(const char* ipString);
	static	ulong	GetIpAddress(const wchar* ipString);
	static	char*	GetIpAddressString(const ulong ip);
	static	void	GetIpAddressString(const ulong ip, char* ipString);
	static	void	GetIpAddressString(const ulong ip, wchar* ipString);
	static	bool	IsValidIp(const wchar* ipString);

private:
	bool	InitSocketInterface();
	int		FindFreePortIndex();

private:
	HANDLE				_iocp;
	ThreadManager*		_pThreadManager;
	int					_threadCount;
	NetworkEndEvent*	_pEndEvent;
	bool				_run;
	uint32*				_threadIds;
	Port*				_pPorts[MAX_PORT_COUNT];

	static	bool						IsInitSocketInterface;
	static	LPFN_ACCEPTEX				lpfnAcceptEx;
	static	LPFN_GETACCEPTEXSOCKADDRS	lpfnGetAcceptExSockAddrs;
	static	LPFN_CONNECTEX				lpfnConnectEx;
	static	LPFN_DISCONNECTEX			lpfnDisconnectEx;
	static	TrustedPacketBufferHandler	DefaultPacketBufferHandler;
};

