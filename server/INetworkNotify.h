#pragma once

#include "Define.h"

class Socket;
class Packet;

class INetworkNotify
{
public:
	virtual ~INetworkNotify(void) {}
	virtual	bool	OnAccept(Socket* pSocket, uint32 portId) = 0;
	virtual	void	OnConnect(Socket* pSocket, bool bSuccess) = 0;
	virtual	void	OnClose(Socket* pSocket) = 0;
	virtual	void	OnReceived(Socket* pSocket, Packet& packet) = 0;
};