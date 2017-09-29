#include "Socket.h"
#include "Network.h"
#include "Port.h"
#include "INetworkNotify.h"

	static	const size_t	SOCKET_OFFSET	= sizeof(Socket);

	// ReconnectTask
// 	class ReconnectTask : public TaskBase
// 	{
// 	public:
// 		ReconnectTask(Port* pPort, Socket::ID socketId)
// 			: TaskBase(true), _pPort(pPort), _socketId(socketId) {}
// 		virtual ~ReconnectTask() {}
// 
// 		uint64	TaskId() const override	{	return 0;	}
// 		bool	Dispatch() override
// 		{
// 			if (!_pPort)
// 				return false;
// 
// 			_pPort->ReconnectSocket(_socketId);
// 			return true;
// 		}
// 	protected:
// 		void	Delete() override
// 		{
// 			delete this;
// 		}
// 
// 	private:
// 		Port*		_pPort;
// 		Socket::ID	_socketId;
// 
// 		friend class Socket;
// 	};

	// SocketRecvBuffer
	SocketRecvBuffer::SocketRecvBuffer()
		: _recvdSize(0)
	{
		memset(_buffer, 0, sizeof(_buffer));
		memset(&_context, 0, sizeof(_context));
		_context.id = EVID_IO_RECV_ID;
	}

	bool SocketRecvBuffer::RemoveRecvdData(size_t size)
	{
		if (0 >= size || PacketBuffer::MaxBuffer < size)
		{
			LOG_DEBUG(L"cannot remove recv buffer, size error, size: %d", size);
			return false;
		}

		if (_recvdSize < size)
		{
			LOG_ERROR(L"cannot remove recv buffer, size error, m_recvdSize:%d,  size: %d", _recvdSize, size);
			return false;
		}

		memmove(_buffer, (_buffer + size), _recvdSize - size);
		_recvdSize	-= size;

		return true;
	}

	size_t SocketRecvBuffer::UpdateRecvdSize(size_t size)
	{
		if (0 >= size || PacketBuffer::MaxBuffer < _recvdSize + size) {
			return 0;
		}
		_recvdSize	+= size;

		return _recvdSize;
	}

	// SocketSendbuffer
	SocketSendbuffer::SocketSendbuffer(Socket* pSocket)
		: _pCurrentBufferList(nullptr)
	{
		memset(_bufferLists, 0, sizeof(_bufferLists));
		memset(&_context, 0, sizeof(_context));
		_context.id = EVID_IO_SEND_ID;
		_pSocket = pSocket;
	}

	SocketSendbuffer::~SocketSendbuffer()
	{
		Clear(true);
	}

	bool SocketSendbuffer::Initialize(byte* pFreeBuffer, size_t freeSize)
	{
		size_t initSize = freeSize / ((sizeof(WSABUF) + sizeof(PacketBuffer*)) * _countof(_bufferLists));

		for (int i = 0; i < _countof(_bufferLists); ++i)
		{
			if (0 != _bufferLists[i].size)
			{
				LOG_ERROR(L"socketsendbuffer initialize error");
				return false;
			}

			_bufferLists[i].external = false;
			_bufferLists[i].count = 0;
			_bufferLists[i].pNext = _bufferLists + ((i+1) % _countof(_bufferLists));
			if (initSize <= 0)
			{
				_bufferLists[i].size = DefaultBufferSize;
				IncreaseBuffer(&(_bufferLists[i]));
			}
			else
			{
				_bufferLists[i].size = (int)initSize;
				_bufferLists[i].pWsaBuffer = (WSABUF*)pFreeBuffer;
				pFreeBuffer += sizeof(WSABUF) * initSize;

				_bufferLists[i].pBuffer = (PacketBuffer**)pFreeBuffer;
				pFreeBuffer += sizeof(PacketBuffer**) * initSize;
			}
		}

		return true;
	}

	AsyncSendContext* SocketSendbuffer::Add(PacketBuffer* pBuffers[], int count)
	{
		AsyncSendContext* pContext = nullptr;

		if (count <= 0 || nullptr == pBuffers)
		{
			LOG_ERROR(L"send parameter is invalid, count: %d, pbuffers: 0x%p", count, pBuffers);
			return nullptr;
		}

		char* buf = nullptr;
		int len = 0;

		SendBufferList* availList = (nullptr == _pCurrentBufferList) ? _bufferLists + 0 : _pCurrentBufferList->pNext;
		for (int i = 0; i < count; ++i)
		{
			PacketBuffer* pBuffer = pBuffers[i];
			while (pBuffer)
			{
				if (availList->size <= availList->count + 1)
				{
					IncreaseBuffer(availList);
				}

				pBuffer->AddRef();

				len = _pSocket->GetPacketBufferHandler()->ToBuffer(nullptr
					, PacketBuffer::MaxBuffer, pBuffer->GetSendBuffer(), (const byte**)&buf);

				availList->pWsaBuffer[availList->count].len = len;
				availList->pWsaBuffer[availList->count].buf = buf;
				availList->pBuffer[availList->count] = pBuffer;
				++availList->count;

				pBuffer = pBuffer->GetNext();
			}
		}

		if (nullptr == _pCurrentBufferList && 0 < availList->count)
		{
			_context.pBufferList = _pCurrentBufferList = availList;
			pContext = &_context;
		}

		return pContext;
	}

	AsyncSendContext* SocketSendbuffer::RemoveSentBuffer(ulong bytes)
	{
		AsyncSendContext* pContext = nullptr;

		if (nullptr == _pCurrentBufferList)
		{
			LOG_ERROR(L"cannot remove send buffer; curBufferList is NULL, socket id: %I64X, sent-size: %u, context-buffer: 0x%p, context-buffer-count: %d"
				, _pSocket->GetId(), bytes, _context.pBufferList, (nullptr != _context.pBufferList) ? _context.pBufferList->count : 0);
			return nullptr;
		}

		for (int i = 0; i < _pCurrentBufferList->count; ++i)
		{
			if (_pCurrentBufferList->pBuffer[i])
			{
				_pCurrentBufferList->pBuffer[i]->ReleaseRef();
			}

			_pCurrentBufferList->pWsaBuffer[i].len = 0;
			_pCurrentBufferList->pWsaBuffer[i].buf = nullptr;
			_pCurrentBufferList->pBuffer[i] = nullptr;
		}

		_pCurrentBufferList->count = 0;

		if (0 < _pCurrentBufferList->pNext->count)
		{
			_context.pBufferList = _pCurrentBufferList = _pCurrentBufferList->pNext;
			pContext = &_context;
		}
		else {
			_context.pBufferList = _pCurrentBufferList = nullptr;
		}

		return pContext;
	}

	void SocketSendbuffer::Clear(bool reset /* = false */)
	{
		for (int i = 0; i < _countof(_bufferLists); ++i)
		{
			ClearBuffer(&_bufferLists[i]);

			if (reset && _bufferLists[i].external)
			{
				LOG_INFO(L"sendbuffer reset");

				if (_bufferLists[i].pWsaBuffer)
				{
					free(_bufferLists[i].pWsaBuffer);
					_bufferLists[i].pWsaBuffer = nullptr;
				}

				if (_bufferLists[i].pBuffer)
				{
					free(_bufferLists[i].pBuffer);
					_bufferLists[i].pBuffer = nullptr;
				}

				_bufferLists[i].size = 0;
				_bufferLists[i].external = false;
			}
		}

		_pCurrentBufferList = nullptr;
	}

	void SocketSendbuffer::IncreaseBuffer(SendBufferList* pBufferList)
	{
		int size = pBufferList->size;

		if (!pBufferList->external)
		{
			WSABUF* curWsaBuffer = pBufferList->pWsaBuffer;
			PacketBuffer** curPacketBuffer = pBufferList->pBuffer;

			pBufferList->size = 2;
			while (pBufferList->size < (size << 1)) {
				pBufferList->size <<= 1;
			}

			pBufferList->external = true;
			pBufferList->pWsaBuffer = alloc_d(WSABUF, pBufferList->size);
			pBufferList->pBuffer = alloc_d(PacketBuffer*, pBufferList->size);
			if (curWsaBuffer)
				memcpy(pBufferList->pWsaBuffer, curWsaBuffer, sizeof(WSABUF) * size);
			if (curPacketBuffer)
				memcpy(pBufferList->pBuffer, curPacketBuffer, sizeof(PacketBuffer*) * size);

			LOG_INFO(L"external - alloc, sendbuffer: 0x%p, size: %d", pBufferList, pBufferList->size);
		}
		else 
		{
			pBufferList->size <<= 1;
			pBufferList->pWsaBuffer = realloc_d(WSABUF, pBufferList->pWsaBuffer, pBufferList->size);
			pBufferList->pBuffer = realloc_d(PacketBuffer*, pBufferList->pBuffer, pBufferList->size);

			LOG_INFO(L"external - realloc, sendbuffer: 0x%p, size: %d", pBufferList, pBufferList->size);
		}
	}

	void SocketSendbuffer::ClearBuffer(SendBufferList* pBufferList)
	{
		if (!pBufferList->pBuffer) {
			return;
		}

		for (int i = 0; i < pBufferList->count; ++i)
		{
			if (pBufferList->pBuffer[i])
			{
				pBufferList->pBuffer[i]->ReleaseRef();
				pBufferList->pBuffer[i] = nullptr;
			}
		}

		pBufferList->count = 0;
	}




	// Socket
	Socket::Socket()
	{
		StaticPageMemoryManager allocator;
		allocator.Initialize(this, DEFAULT_BUFFER_SIZE, SOCKET_OFFSET, NULL);
#ifdef _LEAKCHECK_
		_pContext = (AsyncConnectContext*)allocator.Alloc(sizeof(AsyncConnectContext));
		_pRecvBuffer = (SocketRecvBuffer*)allocator.Alloc(sizeof(SocketRecvBuffer));
		_pRecvBuffer->SocketRecvBuffer::SocketRecvBuffer();
		_pSendBuffer = (SocketSendbuffer*)allocator.Alloc(sizeof(SocketSendbuffer));
		_pSendBuffer->SocketSendbuffer::SocketSendbuffer(this);
#else
		_pContext = new ( allocator.Alloc(sizeof(AsyncConnectContext)) ) AsyncConnectContext;
		_pRecvBuffer = new ( allocator.Alloc(sizeof(SocketRecvBuffer)) ) SocketRecvBuffer;
		_pSendBuffer = new ( allocator.Alloc(sizeof(SocketSendbuffer)) ) SocketSendbuffer(this);
#endif

		size_t freeSize = allocator.FreeSize();
		byte* pFreeBuffer = static_cast<byte*>(allocator.Alloc(freeSize));
		_pSendBuffer->Initialize(pFreeBuffer, freeSize);

		Clear();

		_pNext = nullptr;
	}

	Socket::Socket(IPacketBufferHandler* pPacketBufferHandler)
	{
		AddRef();

		Clear();
		_pPacketBufferHandler = pPacketBufferHandler;
		_minPacketSize = pPacketBufferHandler->MinimalPacketSize();
		_pNext = nullptr;
	}

	Socket::Socket(const wchar_t* reason)
	{
		_pSendBuffer->Clear(true);
	}

	Socket::~Socket(void)
	{
// 		if (_pReconnectTask)
// 		{
// 			_pReconnectTask->Release();
// 			_pReconnectTask = nullptr;
// 		}
	}

	void Socket::Clear()
	{
		_id = INVALID_ID;
		_pPort = nullptr;
		_iocp = INVALID_HANDLE_VALUE;
		_option = ACCEPT;
		_state = CLOSED;
		_hSocket = INVALID_SOCKET;
		memset(&_localAddr, 0, sizeof(_localAddr));
		memset(&_peerAddr, 0, sizeof(_peerAddr));
		_connectCount = 0;
		_connectPending = false;
		_recvPending = false;
		_sendPending = false;
		_minPacketSize = 0;
		//_pReconnectTask = nullptr;
		_pSession = nullptr;

		_pRecvBuffer->Clear();
		_pSendBuffer->Clear();
		_pPacketBufferHandler = nullptr;

		memset(_ipString, 0, sizeof(_ipString));
	}

	void Socket::Delete()
	{
		_pPort->ReleaseSocket(this);

		Clear();
	}

	bool Socket::HandleEvent(EventContext *pContext, ulong param)
	{
		bool result = false;

		switch (pContext->id)
		{
		case EVID_IO_CONNECT_ID:
			{
				AsyncConnectContext* pConnectContext = (AsyncConnectContext*)pContext;
				if (!CompleteAsyncConnect(pConnectContext))
				{
					_pPort->GetNetworkNotify()->OnConnect(this, false);
					return false;
				}

				_lock.Enter();
				_state = CONNECTED;
				pConnectContext->timeout = 0;
				++_connectCount;
				_lock.Leave();

				_pPort->GetNetworkNotify()->OnConnect(this, true);

				_lock.Enter();
				result = Recv();
				_lock.Leave();

				if (!result)
				{
					LOG_ERROR(L"first recv() error, id: %I64X", GetId());
					return false;
				}

				AddRef();

				LOG_DEBUG(L"socket connect, id: %I64X, ip: %s, portno: %u", GetId(), _ipString, ntohs(_peerAddr.sin_port));
			}
			break;
		case EVID_IO_RECV_ID:
			{
				if (0 == param)
				{
					LOG_ERROR(L"recv zero; socket has closed, id: %I64X", GetId());
					return false;
				}
				else if (0 == _pRecvBuffer->UpdateRecvdSize(param))
				{
					LOG_ERROR(L"invalid received size, id: %I64X, bytes: %d", GetId(), param);
					return false;
				}
				else if (!HandlePacket())
				{
					LOG_ERROR(L"handle packet failed, id: %I64X", GetId());
					return false;
				}
				else
				{
					_lock.Enter();
					result = Recv();
					_lock.Leave();

					if (!result)
					{
						LOG_ERROR(L"next recv error, id: %I64X, error: %d", GetId(), WSAGetLastError());
						return false;
					}
				}
			}
			break;
		case EVID_IO_SEND_ID:
			{
				_lock.Enter();
				result = SendNext(param);
				_lock.Leave();

				if (!result)
				{
					LOG_ERROR(L"next send error, id: %I64X, error: %d", GetId(), WSAGetLastError());
					return false;
				}
			}
			break;

		default:
			break;
		}

		return true;
	}

	//bool Socket::Connect(ID socketId, Port* pPort, ulong ipaddress, ushort portNo, uint32 connectTimeout /* = DEFAULT_CONNECT_TIMEOUT */, /*bool reconnect /* = true */, bool async /* = true */)
	bool Socket::Connect(ID socketId, Port* pPort, ulong ipaddress, ushort portNo, uint32 connectTimeout, bool async)
	{
		_id = socketId;
		_pPort = pPort;
		_iocp = pPort->GetNetwork()->IocpHandle();
		_connectCount = 0;
		//_option = (reconnect) ? RECONNECT : CONNECT;
		_option = CONNECT;
		_state = CONNECTING;

		memset(_pContext, 0, sizeof(AsyncConnectContext));
		_pContext->id = EVID_IO_CONNECT_ID;
		_pContext->hSocket = INVALID_SOCKET;
		_pContext->remoteAddr.sin_family = AF_INET;
		_pContext->remoteAddr.sin_port = htons(portNo);
		_pContext->remoteAddr.sin_addr.S_un.S_addr = ipaddress;
		_pContext->firstConnect = true;		

		if (0 < connectTimeout)
		{
			//TODO timeout 옵션 설정은 ms 끝나고 시간날 때 하자 by riverstyx
			_pContext->timeout = GetTickCount() + connectTimeout;
		}

		if (async)
		{
			_connectPending = true;
			if (!AsyncConnect())
			{
				_state = CLOSED;
				_connectPending = false;

				_pPort->GetNetworkNotify()->OnConnect(this, false);

				LOG_ERROR(L"initialize connect error, id: %I64X", GetId());
				return false;
			}
		}
		else
		{
			if (!SyncConnect(ipaddress, portNo))
			{
				_state = CLOSED;
				LOG_ERROR(L"initialize connect error, id: %I64X", GetId());

				_pPort->GetNetworkNotify()->OnConnect(this, false);
				return false;
			}

			_pPort->GetNetworkNotify()->OnConnect(this, true);

			if (!Recv())
			{
				_state = CLOSED;
				LOG_ERROR(L"first recv error, id: %I64X", GetId());
				return false;
			}

			AddRef();

			LOG_DEBUG(L"socket connect, id: %I64X, ip: %s, portno: %u", GetId(), _ipString, ntohs(_peerAddr.sin_port));
		}		
		return true;
	}

	void Socket::Close(bool force /* = false */)
	{
		_lock.Enter();
		CloseSocket(force);
		_lock.Leave();
	}

	bool Socket::IsConnected()
	{
		bool connected = false;

		_lock.Enter();
		connected = (CONNECTED == _state && IsValidSocketHandle()) ? true : false;
		_lock.Leave();

		return connected;
	}

	bool Socket::Send(const byte* pBuffer, ulong size)
	{
		OG_ASSERTR(PacketBuffer::MaxBuffer >= size, false, L"packetbuffer overflow, size: %u/%u", size, PacketBuffer::MaxBuffer);

#ifdef _DEBUG
		Packet packet(pBuffer);
		//OG_ASSERTR(packet.GetSize() == size, false, L"invalid packet size, size: %u/%u", packet.GetSize(), size);
#endif

		TAutoLock<CriticalSection> autoLock(_lock);

		PacketBuffer* list[1];
		PacketBuffer* pPacketBuffer = _pPort->GetPacketPool()->Alloc();
		pPacketBuffer->SetBuffer(pBuffer, size);
		list[0] = pPacketBuffer;
		bool result = SendPacketBuffer(list, _countof(list));
		pPacketBuffer->ReleaseRef();

		return result;
	}

	bool Socket::SendPacketBuffer(PacketBuffer* pBuffers[], int count)
	{
		if (CONNECTED != _state && CLOSING != _state || !IsValidSocketHandle())
		{
			LOG_ERROR(L"send error, id: %I64X, state: %d", GetId(), _state);
			CloseSocketHandle();
			return false;
		}

		AsyncSendContext* pContext = _pSendBuffer->Add(pBuffers, count);
		if (pContext)
		{
			if (!AsyncSend(pContext)) {
				CloseSocketHandle();
				return false;
			}

			_sendPending = true;
		}

		return true;
	}

// 	bool Socket::Reconnect()
// 	{
// 		if (!_pPort->GetNetwork() || !_pPort->GetNetwork()->IsRun())
// 		{
// 			LOG_ERROR(L"Network module is terminated; cancel reconnect, id: %I64X", GetId());
// 		}
// 
// 		if (!_pPort->IsRun())
// 		{
// 			LOG_ERROR(L"port is not run; cancel reconnect, id: %I64X", GetId());
// 			return false;
// 		}
// 
// 		_state = CONNECTING;
// 		_connectPending = true;
// 		_pContext->firstConnect = false;
// 		if (!AsyncConnect())
// 		{
// 			_state = CLOSED;
// 			_connectPending = false;
// 			LOG_ERROR(L"reconnect error, id: %I64X", GetId());
// 			return false;
// 		}
// 
// 		return true;
// 	}

// 	void Socket::StopReconnect()
// 	{	
// 		TAutoLock<CriticalSection> autoLock(_lock);
// 		if (RECONNECT == _option && _pReconnectTask)
// 		{
// 			OG_ASSERT( !_pPort && !(_pPort->GetNetwork()) && !(_pPort->GetNetwork()->GetTaskManager()), L"id: %I64X", GetId());
// 			 
// 			TaskManager* pTM = _pPort->GetNetwork()->GetTaskManager();
// 			pTM->RemoveTask(_pReconnectTask);
// 		}		
// 	}

	bool Socket::Accept(ID socketId, Port* pPort, SOCKET s, const sockaddr_in& localAddr, const sockaddr_in& peerAddr)
	{
		_id = socketId;
		_pPort = pPort;
		_iocp = pPort->GetNetwork()->IocpHandle();
		_hSocket = s;
		memcpy(&_localAddr, &localAddr, sizeof(_localAddr));
		memcpy(&_peerAddr, &peerAddr, sizeof(_peerAddr));
		Network::GetIpAddressString(_peerAddr.sin_addr.S_un.S_addr, _ipString);
		_state = CONNECTED;

		if (NULL == CreateIoCompletionPort((HANDLE)s, _iocp, (ULONG_PTR)this, 0))
		{
			LOG_ERROR(L"iocp bind failed, id: %I64X", GetId());
			return false;
		}

		if ( !_pPort->GetNetworkNotify()->OnAccept(this, Socket::GetPortId(_id)) )
		{
			_state = FORCE_CLOSING;
			LOG_ERROR(L"failed to onaccept, id: %I64X", GetId());
			return false;
		}

		if (!Recv())
		{
			LOG_ERROR(L"first recv error, id: %I64X", GetId());
			return false;
		}

		AddRef();

#ifdef _DEBUG
		LOG_DEBUG(L"accept success, id: %I64X, ip: %s, portno: %u", GetId(), _ipString, ntohs(_peerAddr.sin_port));
#endif

		return true;
	}

	SOCKET Socket::MakeSocket(bool nagle, bool zeroBuf /* = false */)
	{
		SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (SOCKET_ERROR == s)
		{
			LOG_ERROR(L"create socket failed with error: %u", WSAGetLastError());
			return INVALID_SOCKET;
		}

		int	optval = 0;
		if (zeroBuf)
		{
			optval = 0;
			setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(optval));
			setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval));
		}

		if (!nagle)
		{
			optval = 1;
			setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval));
		}

		return s;
	}

	bool Socket::AsyncConnect()
	{
		if (INVALID_SOCKET == _pContext->hSocket)
		{
			SOCKET s = MakeSocket(_pPort->UseNagle());
			if (INVALID_SOCKET == s) {
				return false;
			}

			sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family	= AF_INET;
			bind(s, (const sockaddr*)&addr, sizeof(addr));
			if (NULL == CreateIoCompletionPort((HANDLE)s, _iocp, (ULONG_PTR)this, 0))
			{
				LOG_ERROR(L"iocp bind failed, id: %I64X", GetId());
				closesocket(s);
				return false;
			}

			_pContext->hSocket = s;
		}

		ulong sentSize = 0;
		Network::GetConnectExFunc()(_pContext->hSocket, (const sockaddr*)&_pContext->remoteAddr, sizeof(struct sockaddr), NULL, 0, NULL, _pContext);

		return true;
	}

	bool Socket::SyncConnect(ulong ipaddress, ushort portNo)
	{
		SOCKET s = MakeSocket(_pPort->UseNagle(), _pPort->UseZeroSocketBuffer());
		if (INVALID_SOCKET == s)
		{
			return false;
		}		

		sockaddr_in remoteAddr;
		remoteAddr.sin_family = AF_INET;
		remoteAddr.sin_port = htons(portNo);
		remoteAddr.sin_addr.S_un.S_addr = ipaddress;
		if ( SOCKET_ERROR == connect(s, (const sockaddr*)&remoteAddr, sizeof(remoteAddr)) )
		{
			LOG_ERROR(L"connect error, id: %I64X, error: %d", GetId(), WSAGetLastError());
			return false;
		}

		if ( NULL == CreateIoCompletionPort((HANDLE)s, _iocp, (ULONG_PTR)this, 0) )
		{
			LOG_ERROR(L"iocp bind failed, id: %I64X", GetId());
			closesocket(s);
			return false;
		}

		setsockopt(_hSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);

		_hSocket = s;
		int addrSize = sizeof(_localAddr);
		getsockname(_hSocket, (sockaddr*)&_localAddr, &addrSize);
		memcpy(&_peerAddr, &remoteAddr, sizeof(_peerAddr));
		Network::GetIpAddressString(_peerAddr.sin_addr.S_un.S_addr, _ipString);
		_state = CONNECTED;
		
		return true;
	}

	bool Socket::CompleteAsyncConnect(AsyncConnectContext* pContext)
	{
		setsockopt(_hSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);

		_hSocket = pContext->hSocket;
		int addrSize = sizeof(_localAddr);
		getsockname(_hSocket, (sockaddr*)&_localAddr, &addrSize);
		memcpy(&_peerAddr, &pContext->remoteAddr, sizeof(_peerAddr));
		Network::GetIpAddressString(_peerAddr.sin_addr.S_un.S_addr, _ipString);

		pContext->hSocket = INVALID_SOCKET;
		memset(&pContext->remoteAddr, 0, sizeof(pContext->remoteAddr));
		pContext->firstConnect = false;
		pContext->timeout = 0;

		return true;
	}

// 	bool Socket::TryReconnect(uint32 time)
// 	{
// 		if (!_pPort->GetNetwork() || !_pPort->GetNetwork()->IsRun())
// 		{
// 			LOG_ERROR(L"Network module is terminated; cancel reconnect, id: %I64X", GetId());
// 		}
// 
// 		if (RECONNECT != m_option || !_pReconnectTask)
// 		{
// 			LOG_ERROR(L"cannot reconnect session, id: %I64X", GetId());
// 			return false;
// 		}
// 
// 		if (CLOSED != _state)
// 		{
// 			LOG_ERROR(L"connect is alive; cannot reconnect, id: %I64X", GetId());
// 			return false;
// 		}
// 
// 		if (0 < _pContext->timeout && _pContext->timeout <= GetTickCount())
// 		{
// 			LOG_ERROR(L"connect timeout, id: %I64X", GetId());
// 			return false;
// 		}
// 
// 		if (0 == _connectCount)
// 		{
// 			return Reconnect();
// 		}
// 
// 		OG_ASSERTR( !_pPort && !(_pPort->GetNetwork()) && !(_pPort->GetNetwork()->GetTaskManager()), false, L"id: %I64X", GetId());
// 
// 		TaskManager* pTM = _pPort->GetNetwork()->GetTaskManager();
// 		if (pTM->AddTask(_pReconnectTask, time))
// 		{
// 			LOG_DEBUG(L"reconnect task error, id: %I64X", GetId());
// 			return false;
// 		}
// 
// 		return true;
// 	}

	bool Socket::Recv()
	{
		_recvPending = false;

		if (!IsConnected() || !_pRecvBuffer)
			return false;

		AsyncRecvContext* context = _pRecvBuffer->GetContext();
		context->wsaBuffer.buf = reinterpret_cast<char*>(_pRecvBuffer->GetFreeBuffer());
		context->wsaBuffer.len = static_cast<ulong>(_pRecvBuffer->GetFreeBufferSize());

		ulong recvdSize	= 0;
		if (!AsyncRecv(context, &recvdSize)) {
			return false;
		}

		_recvPending = false;

		return true;
	}

	bool Socket::AsyncRecv(AsyncRecvContext* pContext, ulong* pRecvdSize)
	{
		ulong numberOfbytesRecvd = 0;
		ulong flags = 0;
		if (SOCKET_ERROR == WSARecv(_hSocket, &pContext->wsaBuffer, 1, &numberOfbytesRecvd, &flags, pContext, NULL))
		{
			int errorCode = WSAGetLastError();
			if (ERROR_IO_PENDING != errorCode)
			{
				LOG_ERROR(L"recv error, id: %I64X, error: %d", GetId(), errorCode);
				return false;
			}
		}

		return true;
	}

	bool Socket::HandlePacket()
	{
		const byte* pPacketBuffer = nullptr;
		size_t handleSize = 0;
		size_t bufferSize = _pRecvBuffer->GetRecvdSize() - handleSize;
		while (_minPacketSize <= bufferSize)
		{
			size_t packetSize = _pPacketBufferHandler->ToPacket(
				_pRecvBuffer->GetRecvdBuffer() + handleSize, bufferSize, _pRecvBuffer->GetFreeBufferSize(), &pPacketBuffer);
			if (0 > packetSize || Packet::MaxBufferSize < packetSize) {
				return false;
			}

			if (0 == packetSize) {
				break;
			}

			if (_pPort->IsRun()) {
				//Packet packet((char*)pPacketBuffer, (uint16)packetSize);
				Packet packet(pPacketBuffer);
				_pPort->GetNetworkNotify()->OnReceived(this, packet);
			}

			handleSize += packetSize;
			bufferSize = _pRecvBuffer->GetRecvdSize() - handleSize;
		}
		_pRecvBuffer->RemoveRecvdData(handleSize);

		return true;
	}

	void Socket::CloseSocket(bool force)
	{
		if (FORCE_CLOSING != _state && !force)
		{
			_state = CLOSING;
			if (!_sendPending)
			{
				CloseSocketHandle();
#ifdef _DEBUG
				LOG_DEBUG(L"socket has closed, id: %I64X", GetId());
#endif
			}
			else
			{
				LOG_INFO(L"something remain to send, id: %I64X", GetId());
			}
		}
		else
		{
			_state = FORCE_CLOSING;
			CloseSocketHandle();
#ifdef _DEBUG
			LOG_DEBUG(L"socket has closed, id: %I64X", GetId());
#endif
		}

		if (CLOSING != _state &&
			!_recvPending &&
			!_sendPending &&
			!_connectPending &&
			!IsValidSocketHandle())
		{
			_pRecvBuffer->Clear();
			_pSendBuffer->Clear();
			_state = CLOSED;
			
			if (_pPort)
			{
				_pPort->RemoveSocket(this);
			}
			else
			{
				LOG_ERROR(L"port is null");
			}
		}
	}

	void Socket::CloseSocketHandle()
	{
		if (INVALID_SOCKET != _hSocket)
		{
			LINGER ling = {0,};
			ling.l_onoff = 1;
			ling.l_linger = 0;
			setsockopt(_hSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));

			closesocket(_hSocket);
			_hSocket = INVALID_SOCKET;

			ReleaseRef();
		}
	}

	bool Socket::AsyncSend(AsyncSendContext* pContext)
	{
		ulong sentBytes = 0;
		if (SOCKET_ERROR == WSASend(_hSocket, pContext->pBufferList->pWsaBuffer, pContext->pBufferList->count, &sentBytes, 0, pContext, NULL))
		{
			int errorCode = WSAGetLastError();
			if (ERROR_IO_PENDING != errorCode)
			{
				LOG_ERROR(L"send error, id: %I64X, error: %d", GetId(), errorCode);
				return false;
			}
		}

		return true;
	}

	bool Socket::SendNext(ulong bytes)
	{
		_sendPending = false;
		if (CONNECTED != _state && CLOSING != _state || !IsValidSocketHandle())
		{
			return false;
		}

		AsyncSendContext* pContext = _pSendBuffer->RemoveSentBuffer(bytes);
		if (pContext)
		{
			if (!AsyncSend(pContext))
			{
				return false;
			}
			_sendPending = true;
		}
		else if (CLOSING == _state)
		{
			return false;
		}

		return true;
	}

	void Socket::HandleFailedEvent(EventContext* pContext, ulong param)
	{
		switch (pContext->id)
		{
		case EVID_IO_RECV_ID:
			{
				LOG_INFO(L"recv error, id: %I64X, error: %d", GetId(), WSAGetLastError());

				_pPort->GetNetworkNotify()->OnClose(this);
				_lock.Enter();
				_recvPending = false;
				CloseSocket(false);
				_lock.Leave();
			}
			break;

		case EVID_IO_SEND_ID:
			{
				LOG_ERROR(L"send error, id: %I64X, error: %d", GetId(), WSAGetLastError());

				_lock.Enter();
				_sendPending = false;
				_pSendBuffer->Clear();
				CloseSocket(false);
				_lock.Leave();
			}
			break;

		case EVID_IO_CONNECT_ID:
			{
				LOG_ERROR(L"connect error, id: %I64X, error: %d", GetId(), WSAGetLastError());

				_pPort->GetNetworkNotify()->OnConnect(this, false);
				_lock.Enter();
				_connectPending = false;
				CloseSocket(false);
				_lock.Leave();
			}
			break;

		default:
			{
				LOG_ERROR(L"invalid event, id: %u, this: 0x%p", pContext->id, this);
			}
			break;
		}
	}