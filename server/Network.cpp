#include "Network.h"
#include "DataConst.h"
#include "EventObject.h"
#include "StringUtil.h"
#include "NetworkDesc.h"
#include "Port.h"
#include "ThreadManager.h"
#include "IOThread.h"
#include <Ws2tcpip.h>

bool						Network::IsInitSocketInterface = false;
LPFN_ACCEPTEX				Network::lpfnAcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS	Network::lpfnGetAcceptExSockAddrs = nullptr;
LPFN_CONNECTEX				Network::lpfnConnectEx = nullptr;
LPFN_DISCONNECTEX			Network::lpfnDisconnectEx = nullptr;
TrustedPacketBufferHandler	Network::DefaultPacketBufferHandler;


class NetworkEndEvent : public EventObject
{
public:
	NetworkEndEvent() {
		AddRef();
	}
	virtual	uint32	EventId()	const { return EVOBJ_ENDEVENT_ID; }
	virtual	bool	HandleEvent(EventContext *pContext, ulong param) { return 0; }
	void Delete() override
	{
		delete this;
	}

	friend class Network;
};

Network::Network(ThreadManager* pThreadManager)
	: _iocp(INVALID_HANDLE_VALUE)
	, _pThreadManager(pThreadManager)
	, _threadCount(0)
	, _run(false)
	, _threadIds(nullptr)
{
	_pEndEvent = new NetworkEndEvent;
	memset(_pPorts, 0, sizeof(_pPorts));
}

Network::~Network(void)
{
	Finalize();
}

bool Network::Initialize(int32 threadCount, FailedHandleEventFunc pFailedaHandleEvent)
{
	if (!_pThreadManager || 0 >= threadCount)
	{
		return false;
	}

	_threadCount = threadCount;
	_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (INVALID_HANDLE_VALUE == _iocp)
	{
		LOG_ERROR(L"iocp create failed");
		return false;
	}

	if (!IsInitSocketInterface && !InitSocketInterface())
	{
		return false;
	}

	_threadIds = alloc_d(uint32, threadCount);
	for (int32 index = 0; index < threadCount; ++index)
	{
		IOThread* thread = new IOThread;
		thread->Initialize(index + 1, this, pFailedaHandleEvent);
		if (!_pThreadManager->StartThread(thread, &_threadIds[index]))
		{
			LOG_ERROR(L"thread start failed, index: %d", index);
			return false;
		}
	}

	_run = true;

	return true;
}

void Network::Finalize()
{
	_run = false;

	if (_pThreadManager)
	{
		_pThreadManager = nullptr;
	}

	for (int index = 0; index < MAX_PORT_COUNT; ++index)
	{
		if (_pPorts[index])
		{
			_pPorts[index]->Stop();
			_pPorts[index]->Finalize();

			delete _pPorts[index];
			_pPorts[index] = nullptr;
		}
	}

	if (_threadIds)
	{
		free_d(_threadIds);
		_threadIds = nullptr;
	}

	if (_pEndEvent)
	{
		_pEndEvent->ReleaseRef();
		_pEndEvent = nullptr;
	}

	if (INVALID_HANDLE_VALUE != _iocp)
	{
		CloseHandle(_iocp);
		_iocp = INVALID_HANDLE_VALUE;
	}
}

int Network::NewPort(const NetworkDesc &desc, INetworkNotify *pNotify, IPacketBufferHandler *pHandler)
{
	DEBUG_ASSERT_EXPR(INVALID_HANDLE_VALUE != _iocp);

	if (!pHandler)
	{
		pHandler = &DefaultPacketBufferHandler;
	}

	int index = FindFreePortIndex();
	if (index < 0)
	{
		LOG_ERROR(L"port list is full, %d", MAX_PORT_COUNT);
		return InvalidPortId;
	}

	int portId = (index + 1);

	Port* pPort = new Port(portId);
	if (!pPort->Initialize(this, desc.nSockPoolSize, desc.bNagle, desc.bZeroSocketBuffer, pNotify, pHandler)) {
		return 0;
	}

	_pPorts[index] = pPort;

	LOG_INFO(L"new port, port id: %u, pool size: %u, nagle: %s", portId, desc.nSockPoolSize, (desc.bNagle) ? L"true" : L"false");

	return portId;
}

bool Network::Listen(uint32 portId, const wchar *pszIP, const ushort portNo)
{
	uint32 index = GetPortIndex(portId);
	if (0 > index || MAX_PORT_COUNT <= index || !_pPorts[index])
	{
		LOG_ERROR(L"invalid port, port id: %u, ip: %s, portno: %u", portId, pszIP, portNo);
		return false;
	}

	return _pPorts[index]->Listen(pszIP, portNo);
}

bool Network::Connect(uint32 portId, const wchar* pszIP, const ushort portNo, uint32 connectTimeout, bool async /* = true */)
{
	uint32 index = GetPortIndex(portId);
	if (0 > index || MAX_PORT_COUNT <= index || !_pPorts[index])
	{
		LOG_ERROR(L"invalid port, port id: %u, ip: %s, portno: %u", portId, pszIP, portNo);
		return false;
	}

	Socket *pSocket = _pPorts[index]->Connect(pszIP, portNo, connectTimeout, async);
	if (nullptr == pSocket)
	{
		LOG_ERROR(L"cannot make socket, socket is null, port id: %u, ip: %s, portno: %u", portId, pszIP, portNo);
		return false;
	}

	return true;
}

bool Network::ConnectSession(uint32 portId, const wchar *pszIP, const ushort portNo, Session *pSession, uint32 connectTimeout, bool async /* = true */)
{
	uint32 index = GetPortIndex(portId);
	if (0 > index || MAX_PORT_COUNT <= index || !_pPorts[index])
	{
		LOG_ERROR(L"invalid port, port id: %u, ip: %s, portno: %u", portId, pszIP, portNo);
		return false;
	}

	Socket *pSocket = _pPorts[index]->ConnectSession(pszIP, portNo, pSession, connectTimeout, async);
	if (nullptr == pSocket)
	{
		LOG_ERROR(L"cannot make socket, socket is null, port id: %u, ip: %s, portno: %u", portId, pszIP, portNo);
		return false;
	}

	return true;
}

Port* Network::GetPort(uint32 portId)
{
	uint32 index = GetPortIndex(portId);
	if (0 > index || MAX_PORT_COUNT <= index) {
		return nullptr;
	}

	Port *pPort = _pPorts[index];
	return pPort;
}

bool Network::InitSocketInterface()
{
	ulong bytes = 0;
	int result = 0;
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == s)
	{
		LOG_ERROR(L"create socket failed with error: %u", WSAGetLastError());
		return false;
	}

	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &bytes, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		LOG_ERROR(L"WSAIoctl failed with error: %u", WSAGetLastError());
		closesocket(s);
		return false;
	}

	GUID GuidAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptExSockAddrs, sizeof(GuidAcceptExSockAddrs), &lpfnGetAcceptExSockAddrs, sizeof(lpfnGetAcceptExSockAddrs), &bytes, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		LOG_ERROR(L"WSAIoctl failed with error: %u", WSAGetLastError());
		closesocket(s);
		return false;
	}

	GUID GuidConnectEx = WSAID_CONNECTEX;
	result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEx, sizeof(GuidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &bytes, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		LOG_ERROR(L"WSAIoctl failed with error: %u", WSAGetLastError());
		closesocket(s);
		return false;
	}

	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidDisconnectEx, sizeof(GuidDisconnectEx), &lpfnDisconnectEx, sizeof(lpfnDisconnectEx), &bytes, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		LOG_ERROR(L"WSAIoctl failed with error: %u", WSAGetLastError());
		closesocket(s);
		return false;
	}
	closesocket(s);

	return true;
}

int	Network::FindFreePortIndex()
{
	for (int i = 0; i < MAX_PORT_COUNT; i++)
	{
		if (_pPorts[i] == nullptr) return i;
	}

	return -1;
}

bool Network::Startup()
{
	WSADATA data;
	if (0 != WSAStartup(MAKEWORD(2, 2), &data)) {
		return false;
	}

	return true;
}

void Network::Cleanup()
{
	WSACleanup();
}

ulong Network::GetIpAddress(const char* ipString)
{
	//return inet_addr(ipString);

	in_addr addr;
	inet_pton(AF_INET, ipString, &addr.s_addr);
	return addr.s_addr;
}

ulong Network::GetIpAddress(const wchar *ipString)
{
	char ipaddress[MAX_IP_ADDRESS_LEN + 1];
	StringUtil::UnicodeToChar(ipaddress, ipString, sizeof(ipaddress));
	return Network::GetIpAddress(ipaddress);
}

char* Network::GetIpAddressString(const ulong ip)
{
	static char ipString[MAX_IP_ADDRESS_LEN + 1];

	in_addr addr;
	addr.S_un.S_addr = ip;
	sprintf_s(ipString, MAX_IP_ADDRESS_LEN, "%d.%d.%d.%d", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
	return ipString;
}

void Network::GetIpAddressString(const ulong ip, char *ipString)
{
	in_addr addr;
	addr.S_un.S_addr = ip;
	sprintf_s(ipString, MAX_IP_ADDRESS_LEN, "%d.%d.%d.%d", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
}

void Network::GetIpAddressString(const ulong ip, wchar *ipString)
{
	in_addr addr;
	addr.S_un.S_addr = ip;
	wsprintf(ipString, L"%d.%d.%d.%d", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
}

bool Network::IsValidIp(const wchar *ipString)
{
	if (nullptr == ipString)
		return false;

	int len = static_cast<int>(StringUtil::Length(ipString));
	if (MAX_IP_ADDRESS_LEN < len || 7 > len)
		return false;

	const wchar* str = ipString;
	ulong ipaddress = 0;
	int num = 0, dot = 0;
	for (; ; )
	{
		if ('.' == *str)
		{
			if (3 < ++dot) return false;
			ipaddress = 0;
			num = 0;

			++str;
		}
		else
		{
			if (3 < ++num) return false;
			if (0 == iswdigit(*str)) return false;
		}

		ipaddress = ipaddress * 10 + (*str - '0');
		if (254 < ipaddress) return false;

		++str;
		if (0 == *str)
			break;
	}

	if (3 != dot)
		return false;

	return true;
}
