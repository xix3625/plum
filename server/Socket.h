#pragma once

#include "Define.h"
#include "EventObject.h"
#include "PacketBuffer.h"
#include "Lock.h"
#include "DataConst.h"

class Port;
class IPacketBufferHandler;
class Socket;
class ReconnectTask;
class Session;

struct AsyncConnectContext : EventContext
{
	SOCKET		hSocket;
	sockaddr_in	remoteAddr;
	bool		firstConnect;
	uint32		timeout;
};

struct AsyncRecvContext : EventContext
{
	WSABUF		wsaBuffer;
};

struct SendBufferList
{
	bool			external;
	int				size;
	int				count;
	WSABUF*			pWsaBuffer;
	PacketBuffer**	pBuffer;
	SendBufferList*	pNext;
};

struct AsyncSendContext : EventContext
{
	SendBufferList*	pBufferList;
};


class SocketRecvBuffer
{
public:
	SocketRecvBuffer();
	~SocketRecvBuffer() { Clear(); }

	AsyncRecvContext*	GetContext() { return &_context; }
	inline	byte*		GetRecvdBuffer() { return _buffer; }
	inline	size_t		GetRecvdSize() { return _recvdSize; }
	inline	byte*		GetFreeBuffer() { return _buffer + _recvdSize; }
	inline	size_t		GetFreeBufferSize() { return sizeof(_buffer) - _recvdSize; }
	inline	void		Clear() { _recvdSize = 0; }
	bool				RemoveRecvdData(size_t size);
	size_t				UpdateRecvdSize(size_t size);

private:
	AsyncRecvContext	_context;
	byte				_buffer[PacketBuffer::MaxBuffer];
	size_t				_recvdSize;
};

class SocketSendbuffer
{
public:
	static	const uint32	DefaultBufferSize	= 32;

public:
	SocketSendbuffer(Socket* pSocket);
	~SocketSendbuffer();

	bool				Initialize(byte* pFreeBuffer, size_t freeSize);
	AsyncSendContext*	Add(PacketBuffer* pBuffers[], int count);
	AsyncSendContext*	RemoveSentBuffer(ulong bytes);
	void				Clear(bool reset = false);

private:
	void				IncreaseBuffer(SendBufferList* pBufferList);
	void				ClearBuffer(SendBufferList* pBufferList);

private:
	AsyncSendContext	_context;
	SendBufferList		_bufferLists[2];
	SendBufferList*		_pCurrentBufferList;
	Socket*				_pSocket;
};


class Socket : public EventObject
{
public:
	typedef	uint64	ID;
	static	const ID		INVALID_ID = 0UL;
	static	const uint32	DEFAULT_CONNECT_TIMEOUT = 5000;
	static	const uint32	DEFAULT_RECONNECT_INTERVAL = 5000;
	static	const size_t	DEFAULT_BUFFER_SIZE = 4096 * 2 * 2 * 2;

	enum Option {
		ACCEPT,
		CONNECT,
		RECONNECT
	};

	enum State {
		CLOSED = 0,
		CLOSING,
		FORCE_CLOSING,
		CONNECTING,
		CONNECTED,
	};

	static	uint32	GetPortId(uint64 id) { return (uint32)((id & 0xFFFFFFFF00000000LL) >> 32); }
	static	uint32	GetIndex(uint64 id) { return (uint32)(id & 0x00000000FFFFFFFFLL); }
	static	uint64	MakeId(uint32 portId, uint32 index) { return (uint64)portId << 32 | (uint32)index; }

public:
	virtual ~Socket(void);
	inline	ID	GetId()	const { return _id; }
	uint32	GetPortId()	const { return GetPortId(_id); }
	uint32	GetIndex()	const { return GetIndex(_id); }
	inline	Port*	GetPort() { return _pPort; }

	uint32	EventId() const override { return EVOBJ_SOCKET_ID; }
	bool	HandleEvent(EventContext *pContext, ulong param) override;

	//bool	Connect(ID socketId, Port* pPort, ulong ipaddress, ushort portNo, uint32 connectTimeout = DEFAULT_CONNECT_TIMEOUT, bool reconnect = true, bool async = true);
	bool	Connect(ID socketId, Port* pPort, ulong ipaddress, ushort portNo, uint32 connectTimeout, bool async);
	void	Close(bool force = false);
	bool	IsConnected();
	bool	Send(const byte* pBuffer, ulong size);
	bool	SendPacketBuffer(PacketBuffer* pBuffers[], int count);

	//! 소켓 단에서 재연결 처리 하지 않습니다. 네트워크 단이나 응용단에서 하기로 해요.
	//bool	Reconnect();
	//void	StopReconnect();
	IPacketBufferHandler*	GetPacketBufferHandler() { return _pPacketBufferHandler; }

	// ---------------------------------------------------------------------------		
	inline	ulong	GetPeerIp() { return _peerAddr.sin_addr.S_un.S_addr; }
	inline	ushort	GetPeerPort() { return ntohs(_peerAddr.sin_port); }
	inline	ulong	GetLocalIp() { return _peerAddr.sin_addr.S_un.S_addr; }
	inline	ushort	GetLocalPort() { return ntohs(_peerAddr.sin_port); }
	inline	const wchar*	GetPeerIpString() { return _ipString; }

	Session*		GetSession() { return _pSession; }
	void			SetSession(Session* pSession)
	{
		_pSession = pSession;
	}

	inline	void	EnterLock() { _lock.Enter(); }
	inline	void	LeaveLock() { _lock.Leave(); }

protected:
	void	Delete() override;

private:
	Socket();
	Socket(IPacketBufferHandler* pPacketBufferHandler);
	Socket(const wchar* reason);
	bool	Accept(ID socketId, Port* pPort, SOCKET s, const sockaddr_in& localAddr, const sockaddr_in& peerAddr);
	SOCKET	MakeSocket(bool nagle, bool zeroSocketBuf = false);
	bool	AsyncConnect();
	bool	SyncConnect(ulong ipaddress, ushort portNo);
	bool	CompleteAsyncConnect(AsyncConnectContext* pContext);
	//bool	TryReconnect(uint32 time);

	bool	Recv();
	bool	AsyncRecv(AsyncRecvContext* pContext, ulong* pRecvdSize);
	bool	HandlePacket();
	void	CloseSocket(bool force);
	void	CloseSocketHandle();
	bool	AsyncSend(AsyncSendContext* pContext);
	bool	SendNext(ulong bytes);
	void	HandleFailedEvent(EventContext* pContext, ulong param);
	void	Clear();

	inline	bool	IsValidSocketHandle() { return (INVALID_SOCKET != _hSocket); }

private:
	Socket*	_pNext;
	IPacketBufferHandler*	_pPacketBufferHandler;

	uint64	_id;
	Port*	_pPort;
	HANDLE	_iocp;
	Option	_option;
	State	_state;
	SOCKET	_hSocket;
	sockaddr_in	_localAddr;
	sockaddr_in	_peerAddr;
	wchar		_ipString[MAX_IP_ADDRESS_LEN + 1];

	int		_connectCount;
	bool	_connectPending;
	bool	_recvPending;
	bool	_sendPending;
	size_t	_minPacketSize;

	mutable	CriticalSection	_lock;
	AsyncConnectContext*	_pContext;
	SocketRecvBuffer*		_pRecvBuffer;
	SocketSendbuffer*		_pSendBuffer;
	//ReconnectTask*			_pReconnectTask;


	Session*	_pSession;

	friend class Port;			// Accept, Release
	friend class IOThread;		// HandleFailedEvent
	friend class SocketPool;	// Socket(construct), m_pNext, Release
	friend class ServerSideConnection;
	friend class ClientSideConnection;
};