#pragma once
#include "Define.h"

class IPacketBufferHandler
{
public:
	virtual	size_t	AllocSize() const = 0;
	virtual	void	Initialize() = 0;
	virtual	int		MinimalPacketSize() = 0;
	virtual	int		ToPacket(const byte* pBuffer, size_t bufferSize, size_t freeSize, const byte** pPacket) = 0;
	virtual	int		ToBuffer(byte* pBuffer, int bufferSize, const byte* pPacket, const byte** pSendBuffer) const = 0;
};

class TrustedPacketBufferHandler : public IPacketBufferHandler
{
public:
	size_t	AllocSize() const override;
	void	Initialize() override {}
	int		MinimalPacketSize() override;
	int		ToPacket(const byte*pBuffer, size_t bufferSize, size_t freeSize, const byte** pPacket) override;
	int		ToBuffer(byte* pBuffer, int bufferSize, const byte* pPacket, const byte** pSendBuffer) const override;
};
