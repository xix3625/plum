#include "ServerConnection.h"
#include "Network.h"
#include "Port.h"
#include "Socket.h"
#include "IOThread.h"
#include "StringUtil.h"

//ServerConnection
ServerConnection::ServerConnection()
{
}

ServerConnection::~ServerConnection()
{
}




//ServerSideConnection
ServerSideConnection::ServerSideConnection()
	: _maxSocketCount(0), _lastSelected(-1)
	, _isUsedList(nullptr), _pSocketList(nullptr), _socketCount(0)
{
}

ServerSideConnection::~ServerSideConnection()
{
	Finalize();

	if (_isUsedList)
	{
		free_d(_isUsedList);
		_isUsedList = nullptr;
	}

	if (_pSocketList)
	{
		free_d(_pSocketList);
		_pSocketList = nullptr;
	}
}

void ServerSideConnection::Initialize(int maxSocketCount)
{
	_maxSocketCount = maxSocketCount;
	_pSocketList = alloc_d(Socket*, _maxSocketCount);
	_isUsedList = alloc_d(bool, _maxSocketCount);

	memset(_pSocketList, 0, sizeof(Socket*) * _maxSocketCount);
	memset(_isUsedList, 0, sizeof(bool) * _maxSocketCount);
}

void ServerSideConnection::Finalize()
{
	for (int index = 0; index < _maxSocketCount; ++index)
	{
		if (_pSocketList[index] && _pSocketList[index]->IsConnected()) {
			_pSocketList[index]->ReleaseRef();
			_pSocketList[index] = nullptr;
		}
	}
}

Socket* ServerSideConnection::GetSocket()
{
	Socket* pSocket = nullptr;

	_lock.Enter();
	for (int index = 0; index < _maxSocketCount; ++index)
	{
		int selected = (_lastSelected + index) % _maxSocketCount;
		if (_pSocketList[selected] && !_isUsedList[selected])
		{
			if (!_pSocketList[selected]->IsConnected()) {
				//TODO 이러면 안되는데;; by riverstyx
			}

			_lastSelected = selected;
			_isUsedList[selected] = true;
			pSocket = _pSocketList[selected];
			pSocket->AddRef();
			break;
		}
	}
	_lock.Leave();

	return pSocket;
}

void ServerSideConnection::ReleaseSocket(Socket* pSocket)
{
	bool notfound = true;

	_lock.Enter();
	for (int index = 0; index < _maxSocketCount; ++index)
	{
		if (_pSocketList[index] && _pSocketList[index] == pSocket)
		{
			pSocket->ReleaseRef();
			_isUsedList[index] = false;

			notfound = false;
		}
	}

	if (notfound) {
		//TODO 이러면 안되는데;; by riverstyx
		pSocket->ReleaseRef();
	}
	_lock.Leave();
}

bool ServerSideConnection::OnAccept(Socket* pSocket, uint32 portId)
{
	_lock.Enter();
	if (_maxSocketCount <= _socketCount) {
		_lock.Leave();
		return false;
	}

	for (int index = 0; index < _maxSocketCount; ++index)
	{
		if (!_pSocketList[index])
		{
			++_socketCount;
			_isUsedList[index] = false;
			_pSocketList[index] = pSocket;
			pSocket->AddRef();
			break;
		}
	}
	_lock.Leave();

	return true;
}

void ServerSideConnection::OnClose(Socket* pSocket)
{
	_lock.Enter();
	for (int index = 0; index < _maxSocketCount; ++index)
	{
		if (_pSocketList[index] && _pSocketList[index] == pSocket)
		{
			_pSocketList[index]->ReleaseRef();
			_pSocketList[index] = nullptr;
			_isUsedList[index] = false;
			--_socketCount;
			break;
		}
	}
	_lock.Leave();
}




//ClientSideConnection
ClientSideConnection::ClientSideConnection()
	: _pNetwork(nullptr), _portId(0), _portNo(0)
	, _pSocketList(nullptr), _maxSocketCount(0), _socketCount(0)
{
	memset(_ipaddress, 0, sizeof(_ipaddress));
}

ClientSideConnection::~ClientSideConnection()
{
	Finalize();
}

void ClientSideConnection::Initialize(INetwork* pNetwork, uint32 portId, const wchar* ipaddress, ushort portNo)
{
	_pNetwork = pNetwork;
	_portId = portId;
	StringUtil::NCopy(_ipaddress, _countof(_ipaddress), ipaddress, StringUtil::Length(ipaddress));
	_portNo = portNo;
}

void ClientSideConnection::Finalize()
{
	if (_pSocketList)
	{
		for (int index = 0; index < _maxSocketCount; ++index)
		{
			if (_pSocketList[index])
			{
				if (_pSocketList[index]->IsConnected())
				{
					_pSocketList[index]->Close();
				}

				_pSocketList[index]->ReleaseRef();
				_pSocketList[index] = nullptr;
			}
		}

		free_d(_pSocketList);
		_pSocketList = nullptr;
	}
}

bool ClientSideConnection::InitSocket(bool* mainSocket, bool threadLocalSocket)
{
	int threadCount = _pNetwork->GetThreadCount();
	if (threadLocalSocket)
		threadCount = 0;

	Port* pPort = _pNetwork->GetPort(_portId);
	if (!pPort) {
		return false;
	}

	_maxSocketCount = threadCount + 1;
	_pSocketList = alloc_d(Socket*, _maxSocketCount);
	memset(_pSocketList, 0, sizeof(Socket*) * _maxSocketCount);

	if (mainSocket)
		*mainSocket = false;
	for (int i = 0; i < _maxSocketCount; ++i)
	{
		_pSocketList[i] = pPort->Connect(_ipaddress, _portNo, Socket::DEFAULT_CONNECT_TIMEOUT, false);
		if (!_pSocketList[i])
			return false;

		_pSocketList[i]->AddRef();
	}

	if (_pSocketList[0] && mainSocket) {
		*mainSocket = true;
	}

	return true;
}

Socket* ClientSideConnection::GetSocket()
{
	Socket* pSocket = _pSocketList[IOThread::GetIOThreadId()];
	if (!pSocket || !pSocket->IsConnected()) {
		if (!_pSocketList[0])
			return nullptr;
		pSocket = _pSocketList[0];
	}
	pSocket->AddRef();

	return pSocket;
}

void ClientSideConnection::ReleaseSocket(Socket* pSocket)
{
	if (pSocket)
		pSocket->ReleaseRef();
}

void ClientSideConnection::OnConnect(Socket* pSocket, bool bSuccess)
{
	if (bSuccess)
	{
		InterlockedIncrement((volatile long*)&_socketCount);
	}
}

void ClientSideConnection::OnClose(Socket* pSocket)
{
	Port* pPort = pSocket->GetPort();
	if (pPort)
	{
		pPort->RemoveSocket(pSocket->GetId());
		InterlockedDecrement((volatile long*)&_socketCount);
	}
}