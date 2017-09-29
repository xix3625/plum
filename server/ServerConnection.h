#pragma once

#include "INetworkNotify.h"
#include "Lock.h"
#include "DataConst.h"

class INetwork;
class Socket;

class ServerConnection : public INetworkNotify
{
public:
	ServerConnection();
	virtual ~ServerConnection();

	virtual	Socket*	GetSocket() = 0;
	virtual	void	ReleaseSocket(Socket* pSocket) = 0;
};

class ServerSideConnection : public ServerConnection
{
public:
	ServerSideConnection();
	virtual ~ServerSideConnection();
	void	Initialize(int maxSocketCount);
	void	Finalize();

	Socket*	GetSocket() override;
	void	ReleaseSocket(Socket* pSocket) override;

	bool	OnAccept(Socket* pSocket, uint32 portId) override;
	void	OnConnect(Socket* pSocket, bool bSuccess) override { return; }
	void	OnClose(Socket* pSocket) override;

private:
	CriticalSection	_lock;
	int				_maxSocketCount;
	int				_lastSelected;
	bool*			_isUsedList;
	Socket**		_pSocketList;
	int				_socketCount;
};

class ClientSideConnection : public ServerConnection
{
public:
	ClientSideConnection();
	virtual ~ClientSideConnection();
	void	Initialize(INetwork* pNetwork, uint32 portId, const wchar* ipaddress, ushort portNo);
	void	Finalize();
	bool	InitSocket(bool* mainSocket, bool threadLocalSocket);

	Socket*	GetSocket() override;
	void	ReleaseSocket(Socket* pSocket) override;
	int		GetMaxGetSocketCount() { return _maxSocketCount; }
	int		GetSocketCount() { return _socketCount; }

	bool	OnAccept(Socket* pSocket, uint32 portId) override { return false; }
	void	OnConnect(Socket* pSocket, bool bSuccess) override;
	void	OnClose(Socket* pSocket) override;

private:
	INetwork*		_pNetwork;
	uint32			_portId;
	wchar			_ipaddress[MAX_IP_ADDRESS_LEN + 1];
	ushort			_portNo;
	Socket**		_pSocketList;
	int				_maxSocketCount;
	int				_socketCount;
};
