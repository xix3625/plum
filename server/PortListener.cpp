#include "PortListener.h"
#include "Network.h"
#include "PortListener.h"
#include "Port.h"
#include "Socket.h"
#include <MSTcpIP.h> //keepalive

PortListener::PortListener(void)
	: _hSocket(INVALID_SOCKET), _pPort(nullptr)
	, _ip(0), _portNo(0)
{
	AddRef();
	memset(&_context, 0, sizeof(_context));
}

PortListener::~PortListener(void)
{
	SOCKET s = InterlockedExchange64(reinterpret_cast<volatile __int64*>(&_hSocket), INVALID_SOCKET);
	if (INVALID_SOCKET != s)
	{
		closesocket(s);
	}

	_pPort = nullptr;
}

bool PortListener::Initialize(Port* pPort, ulong ip, ushort portNo, bool nagle)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == s)
	{
		LOG_ERROR(L"Create accept socket failed with error: %u", WSAGetLastError());
		return false;
	}

	if (NULL == CreateIoCompletionPort((HANDLE)s, pPort->GetNetwork()->IocpHandle(), (ULONG_PTR)this, 0))
	{
		LOG_ERROR(L"iocp bind failed");
		Finalize();
		return false;
	}

	sockaddr_in	addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portNo);
	addr.sin_addr.S_un.S_addr = ip;
	if (SOCKET_ERROR == bind(s, (SOCKADDR*)&addr, sizeof(addr))) {
		LOG_ERROR(L"bind failed with error: %u", WSAGetLastError());
		Finalize();
		return false;
	}

	if (!pPort->UseNagle())
	{
		int optVal = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&optVal, sizeof(optVal));
	}

	if (SOCKET_ERROR == listen(s, CONTEXT_COUNT))
	{
		LOG_ERROR(L"listen failed with error: %u", WSAGetLastError());
		Finalize();
		return false;
	}

	_hSocket = s;
	_pPort = pPort;
	_ip = ip;
	_portNo = portNo;
	for (int i = 0; i < CONTEXT_COUNT; ++i)
	{
		_context[i].id = EVID_IO_ACCEPT_ID;
		_context[i].index = i;
		_context[i].hSocket = INVALID_SOCKET;
		//AcceptEx 바로 연결 by riverstyx
		_context[i].recvedSize = 0;
		//_context[i].recvedSize = CONTEXT_BUFFER - ((sizeof(sockaddr_in) + 16) * 2);
		_context[i].localAddr = sizeof(sockaddr_in) + 16;
		_context[i].remoteAddr = sizeof(sockaddr_in) + 16;
		if (!AsyncAccept(i))
		{
			Finalize();
			return false;
		}
	}

	return true;
}

void PortListener::Finalize()
{
}

bool PortListener::HandleEvent(EventContext* pContext, ulong param)
{
	if (!pContext || EVID_IO_ACCEPT_ID != pContext->id)
	{
		LOG_ERROR(L"invalid eventobject, eventcontext: 0x%p, id: %u", pContext, (pContext) ? pContext->id : EVID_INVALID_ID);
		return false;
	}

	AcceptContext* pAcceptContext = (AcceptContext*)pContext;
	if (!Accept(pAcceptContext))
	{
		closesocket(pAcceptContext->hSocket);
	}
	pAcceptContext->hSocket = INVALID_SOCKET;
	memset(pAcceptContext->buffer, 0, CONTEXT_BUFFER);
	AsyncAccept(pAcceptContext->index);

	return true;
}

bool PortListener::AsyncAccept(uint32 index)
{
	AcceptContext* pContext = &_context[index];
	if (INVALID_SOCKET == pContext->hSocket)
	{
		if (INVALID_SOCKET == (pContext->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
		{
			LOG_ERROR(L"Create accept socket failed with error: %u", WSAGetLastError());
			return false;
		}
	}

	ulong bytesReceived = 0;
	if (FALSE == Network::GetAcceptExFunc()(_hSocket, pContext->hSocket, pContext->buffer
		, pContext->recvedSize, pContext->localAddr, pContext->remoteAddr, &bytesReceived, pContext))
	{
		if (ERROR_IO_PENDING != WSAGetLastError())
		{
			closesocket(pContext->hSocket);
			pContext->hSocket = INVALID_SOCKET;
			LOG_ERROR(L"AcceptEx failed with error: %u", WSAGetLastError());
			return false;
		}
	}

	return true;
}

bool PortListener::Accept(AcceptContext* pContext)
{
	if (!_pPort->EnableAccept())
	{
		LOG_ERROR(L"port, accept is not enable, port id: %u", _pPort->GetId());
		return false;
	}

	setsockopt(pContext->hSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char*)&_hSocket, sizeof(SOCKET));

	// keep-alive 위한 설정
	ULONG bytesReturned = 0;
	struct tcp_keepalive keepAlive, outKeepAlive;
	{
		keepAlive.onoff = 1;
		keepAlive.keepaliveinterval = 5000;
		keepAlive.keepalivetime = 5000;
	}
	WSAIoctl(pContext->hSocket, SIO_KEEPALIVE_VALS, &keepAlive, sizeof(keepAlive), &outKeepAlive, sizeof(outKeepAlive), &bytesReturned, NULL, NULL);

	if (_pPort->UseZeroSocketBuffer())
	{
		int	optval = 0;
		setsockopt(pContext->hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(optval));
		setsockopt(pContext->hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval));
	}

	int addrSize = 0;
	sockaddr_in* localAddr = nullptr;
	sockaddr_in* remoteAddr = nullptr;
	Network::GetAcceptExSockAddrsFunc()(pContext->buffer
		, pContext->recvedSize
		, pContext->localAddr, pContext->remoteAddr
		, (sockaddr**)&localAddr, &addrSize
		, (sockaddr**)&remoteAddr, &addrSize);

	sockaddr_in peerAddr, socketAddr;
	if (0 == pContext->recvedSize || 0 == addrSize)
	{
		addrSize = sizeof(peerAddr);
		getsockname(pContext->hSocket, (sockaddr*)&socketAddr, &addrSize);
		getpeername(pContext->hSocket, (sockaddr*)&peerAddr, &addrSize);
		localAddr = &socketAddr;
		remoteAddr = &peerAddr;
	}

	Socket* pSocket = _pPort->Accept(pContext->hSocket, *localAddr, *remoteAddr);
	if (!pSocket)
	{
		return false;
	}

	return true;
}