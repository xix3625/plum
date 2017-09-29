#pragma once

#include "Define.h"
#include "Lock.h"
#include "MemoryManager.h"

class INetworkNotify;
class IPacketBufferHandler;
class INetwork;
class PortListener;
class Socket;
class PacketBufferPool;
class Session;

#ifdef REF_SOCKETPOOL
class SocketPool : public RefCountObject
#else
class SocketPool
#endif
{
public:
	static	const uint32	PREPARE_RATIO = 100;

public:
	SocketPool(uint32 portId);
	~SocketPool();
#ifdef REF_SOCKETPOOL
	void	Delete() override;
#endif

	bool	Initialize(int maxSocket, int prepareRatio);
	void	Finalize();
	Socket*	MakeSocket(IPacketBufferHandler* pPacketBufferHandler);
	void	RemoveSocket(Socket* pSocket);

private:
	void	AddToPool(Socket* pSocket);

private:
	uint32	_portId;
	bool	_valid;
	Socket*	_pHead;
	Socket*	_pTail;
	PagePoolMemoryManager	_allocator;
	CriticalSection	_lock;

#ifdef REF_SOCKETPOOL
	friend class Port;
#endif
};


class Port
{
public:
	Port(uint32 id);
	~Port(void);

	// ---------------------------------------------------------------------------
	//!
	bool	Initialize(INetwork* pNetwork, uint32 maxSocketCount, bool bNagle, bool zeroSocketBuffer, INetworkNotify* pNetworkNotify, IPacketBufferHandler* pPacketBufferHandler);
	void	Finalize();

	// ---------------------------------------------------------------------------
	//!  
	bool	Listen(const WCHAR* pszIP, ushort portNo);
	Socket* Connect(const WCHAR* pszIP, ushort portNo, uint32 connectTimeout, bool async = true);
	Socket* ConnectSession(const WCHAR* pszIP, ushort portNo, Session* pSession, uint32 connectTimeout, bool async = true);

	void	Open() { _enableAccept = true; }
	void	Close() { _enableAccept = false; }
	void	Stop() { _enableAccept = false; _enableRecv = false; }

	Socket*	Accept(SOCKET s, const sockaddr_in& localAddr, const sockaddr_in& peerAddr);

	bool	RemoveSocket(Socket* pSocket);
	bool	RemoveSocket(uint64 socketId);
	void	CloseSocket(uint64 socketId);

	//void	ReconnectSocket(uint64 socketId);

	inline	INetworkNotify*		GetNetworkNotify() { return _pNetworkNotify; }
	inline	uint32				GetId() { return _id; }
	inline	INetwork*			GetNetwork() { return _pNetwork; }
	inline	bool				UseNagle() { return _nagle; }
	inline	bool				UseZeroSocketBuffer() { return _zeroSocketBuf; }
	inline	bool				IsOpen() { return _enableAccept; }
	inline	bool				IsRun() { return _enableRecv; }
	inline	bool				EnableAccept() { return _enableAccept; }
	inline	PacketBufferPool*	GetPacketPool() { return _pPacketPool; }

private:
	Socket*	MakeSocket(uint64& socketId);
	void	ReleaseSocket(Socket* pSocket);

private:
	uint32				_id;
	INetworkNotify*		_pNetworkNotify;
	IPacketBufferHandler*	_pPacketBufferHandler;
	PortListener*		_pListener;
	uint32				_maxSocketCount;
	INetwork*			_pNetwork;
	bool				_nagle;
	bool				_zeroSocketBuf;
	bool				_enableAccept;
	bool				_enableRecv;

	// socketpool
	CriticalSection		_socketLock;
	SocketPool*			_pSocketPool;
	uint32				_socketCount;
	Socket**			_pSocketList;
	uint32				_unusedSocketIndex;
	// packetpool
	PacketBufferPool*	_pPacketPool;

	friend class Socket;	// ReleaseSocket			
};