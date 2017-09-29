#pragma once

#include "Define.h"
#include "Packet.h"
#include "BinaryBuilder.h"

class PacketBuilder : public BinaryBuilder
{
public:
	PacketBuilder(Packet::ProtocolType protocol = 0);
	PacketBuilder(byte* pBuffer, int64 bufferSize);
	virtual ~PacketBuilder();
	bool	Append(const PacketBuilder& packetBuilder);


	const Packet::PacketSizeType	GetSize();
	void	SetSize(const Packet::PacketSizeType size);
	void	UpdateSize(const Packet::PacketSizeType size = 0);
};

