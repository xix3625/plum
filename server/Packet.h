#pragma once

#include "TypeDefine.h"
#include "ISerializable.h"

class Packet
{
public:
	typedef uint16	PacketSizeType;
	typedef uint16	ProtocolType;

	static const int MaxBufferSize = 1024 * 8;
	static const int LengthSize = sizeof(PacketSizeType);
	static const int ProtocolSize = sizeof(ProtocolType);
	static const int HeaderSize = LengthSize + ProtocolSize;

	static PacketSizeType	GetSize(const byte* pBuffer);

public:
	Packet(const byte* pData);
	~Packet();
	bool	IsValidData();

	inline	const byte*		GetData()		{	return _pData;		}
	inline	PacketSizeType	GetDataSize()	{	return _pDataSize;	}

private:
	PacketSizeType	GetSize();

private:
	const byte*		_pData;
	PacketSizeType	_pDataSize;
};

