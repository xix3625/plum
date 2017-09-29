#include "Port.h"
#include "Socket.h"
#include "Network.h"
#include "PortListener.h"
#include "Socket.h"

// SocketPool
SocketPool::SocketPool(uint32 portId)
	: _portId(portId), _valid(false), _pHead(nullptr), _pTail(nullptr)
{
#ifdef REF_SOCKETPOOL
	AddRef();
#endif
}

SocketPool::~SocketPool()
{
	if (_valid)
	{
		LOG_ERROR(L"socket pool, not cleared, port id: %u", _portId);
		Finalize();
	}
}

#ifdef REF_SOCKETPOOL
void SocketPool::Delete()
{
	Finalize();

	delete this;
}
#endif

bool SocketPool::Initialize(int maxSocket, int prepareRatio)
{
	if (_valid)
	{
		LOG_ERROR(L"socket pool, pool is already initialized, port id: %u", _portId);
		return false;
	}

	bool result = _allocator.Initialize(Socket::DEFAULT_BUFFER_SIZE);
	DEBUG_ASSERT_EXPR(result, L"allocator Initialize failed ");

	int	prepare = maxSocket * prepareRatio / 100;

	Socket* pSocket = nullptr;
	for (int i = 0; i < prepare; ++i)
	{
#ifdef _LEAKCHECK_
		pSocket = (Socket*)_allocator.Alloc();
		pSocket->Socket::Socket();
#else
		pSocket = new (m_allocator.Alloc()) Socket;
#endif
		AddToPool(pSocket);
	}
	_valid = true;

	return true;
}

void SocketPool::Finalize()
{
	_lock.Enter();

	while (_pHead)
	{
		Socket* pSocket = _pHead;
		_pHead = pSocket->_pNext;
		pSocket->Socket::Socket(L"for finalize");
		pSocket->~Socket();
	}

	_allocator.Finalize();

	_pHead = _pTail = nullptr;
	_valid = false;
	_lock.Leave();
}

Socket*	SocketPool::MakeSocket(IPacketBufferHandler* pPacketBufferHandler)
{
	Socket* pSocket = nullptr;

	_lock.Enter();
	if (!_valid)
	{
		_lock.Leave();
		LOG_ERROR(L"socket pool, cannot make socket, port id: %u", _portId);
		return nullptr;
	}

	if (_pHead)
	{
		pSocket = _pHead;
		_pHead = pSocket->_pNext;
		if (!_pHead) {
			_pTail = nullptr;
		}
	}
	else
	{
		//pSocket = new (m_allocator.Alloc()) Socket;
		//pool size를 여유 있게 잡는다, 위 경우는 허용 안함
		pSocket = nullptr;
	}
	_lock.Leave();

	if (!pSocket)
	{
		LOG_ERROR(L"socket pool, cannot make socket, port id: %u", _portId);
		return nullptr;
	}

	//LOG_DEBUG(L"socket pool, make socket, port id: %u, socket: 0x%p", m_portId, pSocket);
	pSocket->Socket::Socket(pPacketBufferHandler);
#ifdef REF_SOCKETPOOL
	AddRef();
#endif

	return pSocket;
}

void SocketPool::RemoveSocket(Socket* pSocket)
{
	if (nullptr == pSocket)
	{
		LOG_ERROR(L"socket pool, invalid socket in removing, port id: %u", _portId);
		return;
	}

	pSocket->~Socket();
	AddToPool(pSocket);
#ifdef REF_SOCKETPOOL
	Release();
#endif

	//LOG_DEBUG(L"socket pool, remove socket, port id: %u, socket: 0x%p", m_portId, pSocket);
}

void SocketPool::AddToPool(Socket* pSocket)
{
	_lock.Enter();
	if (_pTail)
	{
		_pTail->_pNext = pSocket;
	}
	else
	{
		_pHead = pSocket;
	}
	_pTail = pSocket;
	_lock.Leave();
}




// Port
Port::Port(uint32 id)
	: _id(id)
	, _pNetworkNotify(nullptr)
	, _pPacketBufferHandler(nullptr)
	, _pListener(nullptr)
	, _maxSocketCount(0)
	, _pNetwork(nullptr)
	, _nagle(false)
	, _zeroSocketBuf(false)
	, _enableAccept(false)
	, _enableRecv(false)
	, _pSocketPool(nullptr)
	, _socketCount(0)
	, _pSocketList(nullptr)
	, _unusedSocketIndex(0)
	, _pPacketPool(nullptr)
{
}

Port::~Port(void)
{
	Finalize();
}

bool Port::Initialize(INetwork* pNetwork, uint32 maxSocketCount, bool bNagle, bool zeroSocketBuffer, INetworkNotify* pNetworkNotify, IPacketBufferHandler* pPacketBufferHandler)
{
	_pNetwork = pNetwork;

	_maxSocketCount = maxSocketCount + 1;
	_nagle = bNagle;
	_zeroSocketBuf = zeroSocketBuffer;

	_pNetworkNotify = pNetworkNotify;
	_pPacketBufferHandler = pPacketBufferHandler;

	_enableAccept = false;
	_enableRecv = false;

	uint32 socketPrepareRatio = SocketPool::PREPARE_RATIO;

	_pSocketPool = new SocketPool(_id);
	_pSocketPool->Initialize(_maxSocketCount - 1, socketPrepareRatio);
	_unusedSocketIndex = 1;
	_pSocketList = alloc_d(Socket*, _maxSocketCount);
	for (uint32 index = 0; index < _maxSocketCount; ++index)
	{
		_pSocketList[index] = nullptr;
	}

	_pPacketPool = new PacketBufferPool;
	_pPacketPool->Initialize(PacketBufferPool::DefaultSlotSize);

#ifdef _DEBUG
	static bool checkBufferSize = false;
	if (!checkBufferSize)
	{
		size_t bufferSize = 0;
		bufferSize += GetAllocatedSize(sizeof(Socket));
		bufferSize += GetAllocatedSize(sizeof(AsyncConnectContext));
		bufferSize += GetAllocatedSize(sizeof(SocketRecvBuffer));
		bufferSize += GetAllocatedSize(sizeof(SocketSendbuffer));

		LOG_DEBUG(L"check socket buffer size: %d, socket: %d, context: %d, redvbuffer: %d, sendbuffer: %d", bufferSize
			, GetAllocatedSize(sizeof(Socket))
			, GetAllocatedSize(sizeof(AsyncConnectContext))
			, GetAllocatedSize(sizeof(SocketRecvBuffer))
			, GetAllocatedSize(sizeof(SocketSendbuffer))
		);

		checkBufferSize = true;
	}
#endif

	return true;
}

void Port::Finalize()
{
	if (_pSocketList)
	{
		for (uint32 index = 0; index < _maxSocketCount; ++index)
		{
			if (_pSocketList[index])
			{
				LOG_DEBUG(L"port, port id: %u, socket: 0x%p", _id, _pSocketList[index]);
				if (_pSocketList[index]->IsConnected())
				{
					_pSocketList[index]->Close(true);
				}
				_pSocketList[index] = nullptr;
			}
		}

		free_d(_pSocketList);
		_pSocketList = nullptr;
	}

	if (_pListener)
	{
		Stop();

		_pListener->Finalize();
		_pListener->ReleaseRef();
		_pListener = nullptr;
	}

	if (_pSocketPool)
	{
#ifdef REF_SOCKETPOOL
		_pSocketPool->Release();
#else
		_pSocketPool->Finalize();
		delete _pSocketPool;
#endif
		_pSocketPool = nullptr;
	}

	if (_pPacketPool)
	{
		_pPacketPool->Finalize();
		delete _pPacketPool;
		_pPacketPool = nullptr;
	}
}

bool Port::Listen(const WCHAR* pszIP, ushort portNo)
{
	if (!Network::IsValidIp(pszIP))
	{
		LOG_ERROR(L"invalid ip, port id: %u, ip: %s", _id, pszIP);
		return false;
	}

	ulong ip = htonl(INADDR_ANY);
	if (pszIP)
	{
		ip = Network::GetIpAddress(pszIP);
	}

	_pListener = new PortListener;
	if (!_pListener->Initialize(this, ip, portNo, _nagle)) return false;

	LOG_INFO(L"listen port, port id: %d, ip: %s, portno: %u", _id, pszIP, portNo);

	_enableAccept = true;
	_enableRecv = true;

	return true;
}

Socket* Port::Connect(const WCHAR* pszIP, ushort portNo, uint32 connectTimeout, bool async /* = true */)
{
	if (!Network::IsValidIp(pszIP))
	{
		LOG_ERROR(L"invalid ip, port id: %u, ip: %s", _id, pszIP);
		return nullptr;
	}

	Socket::ID socketId = Socket::INVALID_ID;
	Socket* socket = MakeSocket(socketId);
	if (!socket)
	{
		return nullptr;
	}

	if (!socket->Connect(socketId, this, Network::GetIpAddress(pszIP), portNo, connectTimeout, async))
	{
		RemoveSocket(socket);
		return nullptr;
	}

	_enableRecv = true;

	return socket;
}

Socket* Port::ConnectSession(const WCHAR* pszIP, ushort portNo, Session* pSession, uint32 connectTimeout, bool async /* = true */)
{
	if (!Network::IsValidIp(pszIP))
	{
		LOG_ERROR(L"invalid ip, port id: %u, ip: %s", _id, pszIP);
		return nullptr;
	}

	Socket::ID socketId = Socket::INVALID_ID;
	Socket* socket = MakeSocket(socketId);
	if (!socket)
	{
		return nullptr;
	}

	socket->SetSession(pSession);
	if (!socket->Connect(socketId, this, Network::GetIpAddress(pszIP), portNo, connectTimeout, async))
	{
		RemoveSocket(socket);
		return nullptr;
	}

	_enableRecv = true;

	return socket;
}

Socket* Port::Accept(SOCKET s, const sockaddr_in& localAddr, const sockaddr_in& peerAddr)
{
	Socket::ID socketId = Socket::INVALID_ID;
	Socket* pSocket = MakeSocket(socketId);
	if (!pSocket)
	{
		return nullptr;
	}

	if (!pSocket->Accept(socketId, this, s, localAddr, peerAddr))
	{
		LOG_ERROR(L"initialize socket error");
		return nullptr;
	}

	return pSocket;
}

bool Port::RemoveSocket(Socket* pSocket)
{
	uint32 index = pSocket->GetIndex();
	OG_ASSERTR(index < _maxSocketCount, false, L"fatal error, index: %u", index);

	_socketLock.Enter();

	Socket* found = _pSocketList[index];
	if (pSocket == found)
	{
		_pSocketList[index] = nullptr;
		--_socketCount;
		found->ReleaseRef();
	}
	else
	{
		found = nullptr;
	}

	_socketLock.Leave();

	if (nullptr == found) {
		return false;
	}

#ifdef _DEBUG
	LOG_DEBUG(L"remove socket, port id: %u, socket: 0x%p, socket id: %I64X", _id, found, Socket::MakeId(_id, index));
#endif

	return true;
}

bool Port::RemoveSocket(uint64 socketId)
{
	uint32 index = Socket::GetIndex(socketId);
	OG_ASSERTCRASHR(Socket::GetPortId(socketId) == _id, false, L"invalid portid: %u/%u", Socket::GetPortId(socketId), _id);
	OG_ASSERTCRASHR(index < _maxSocketCount, false, L"index overflow, index: %d, maxsocketcount: %u", index, _maxSocketCount);

	_socketLock.Enter();

	Socket* found = _pSocketList[index];
	if (nullptr != found && found->GetId() == socketId)
	{
		_pSocketList[index] = nullptr;
		--_socketCount;
		found->ReleaseRef();
	}
	else
	{
		found = nullptr;
	}

	_socketLock.Leave();

	if (nullptr == found)
	{
		return false;
	}

#ifdef _DEBUG
	LOG_DEBUG(L"remove socket, port id: %u, socket: 0x%p, socket id: %I64X", _id, found, Socket::MakeId(_id, index));
#endif

	return true;
}

void Port::CloseSocket(uint64 socketId)
{
	uint32 index = Socket::GetIndex(socketId);
	OG_ASSERTCRASH(Socket::GetPortId(socketId) == _id, L"invalid portid: %u/%u", Socket::GetPortId(socketId), _id);
	OG_ASSERTCRASH(index < _maxSocketCount, L"index overflow, index: %d, maxsocketcount: %u", index, _maxSocketCount);

	_socketLock.Enter();
	Socket* pSocket = _pSocketList[index];
	if (nullptr != pSocket && pSocket->GetId() == socketId) {
		pSocket->Close();
	}
	_socketLock.Leave();
}

// 	void Port::ReconnectSocket(uint64 socketId)
// 	{
// 		uint32 index = Socket::GetIndex(socketId);
// 
// 		OG_ASSERTCRASH(Socket::GetPortId(socketId) == _id, L"invalid portid: %u/%u", Socket::GetPortId(socketId), _id);
// 		OG_ASSERTCRASH(index < _maxSocketCount, L"index overflow, index: %d, maxsocketcount: %u", index, _maxSocketCount);
// 
// 		_socketLock.Enter();
// 		SocketPtr socketPtr = _pSocketList[index];
// 		if (NULL != socketPtr && socketPtr->GetId() == socketId && !socketPtr->IsConnected())
// 		{
// 			_socketLock.Leave();
// 			return;
// 		}
// 		_socketLock.Leave();
// 
// 		if (!socketPtr)
// 			return;
// 
// 		if (!socketPtr->Reconnect())
// 		{
// 			RemoveSocket(socketPtr.get());
// 			return;
// 		};
// 	}

Socket*	Port::MakeSocket(uint64& socketId)
{
	if (_socketCount >= _maxSocketCount)
	{
		LOG_ERROR(L"socket list is full, %d", _maxSocketCount - 1);
		return nullptr;
	}

	uint32 count = 0;
	Socket* pSocket = nullptr;
	pSocket = _pSocketPool->MakeSocket(_pPacketBufferHandler);
	if (!pSocket)
	{
		LOG_ERROR(L"cannot find empty slot in socketlist, port id: %d", _id);
		return nullptr;
	}

	socketId = Socket::INVALID_ID;

	_socketLock.Enter();

	// find empty slot
	while (count < _maxSocketCount)
	{
		if (0 != _unusedSocketIndex && !_pSocketList[_unusedSocketIndex])
		{
			_pSocketList[_unusedSocketIndex] = pSocket;

			socketId = Socket::MakeId(_id, _unusedSocketIndex);
			++_socketCount;

			_unusedSocketIndex = (_unusedSocketIndex + 1) % _maxSocketCount;
			break;
		}

		_unusedSocketIndex = (_unusedSocketIndex + 1) % _maxSocketCount;
		++count;
	}

	_socketLock.Leave();

	if (Socket::INVALID_ID == socketId)
	{
		LOG_ERROR(L"cannot find empty slot in socketlist, port id: %d", _id);
		return nullptr;
	}
	else
	{
#ifdef _DEBUG
		LOG_DEBUG(L"created, port id: %u, socket: 0x%p, socket id: %I64X", _id, pSocket, socketId);
#endif
		//LOG_DEBUG(L"port id: %u, maxsocket size: %d, allocated socket count: %d", _id, _maxSocketCount - 1, _socketCount);
	}

	return pSocket;
}

void Port::ReleaseSocket(Socket* pSocket)
{
	if (pSocket)
	{
		_pSocketPool->RemoveSocket(pSocket);
	}
}