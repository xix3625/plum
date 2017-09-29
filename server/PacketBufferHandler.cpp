#include "PacketBufferHandler.h"
#include "PacketBuilder.h"

size_t TrustedPacketBufferHandler::AllocSize() const
{
	return sizeof(TrustedPacketBufferHandler);
}

int TrustedPacketBufferHandler::MinimalPacketSize()
{
	return Packet::HeaderSize;
}

int TrustedPacketBufferHandler::ToPacket(const byte* pBuffer, size_t bufferSize, size_t freeSize, const byte** pPacket)
{
	if (MinimalPacketSize() > bufferSize)
		return 0;

	Packet packet(pBuffer);
	if (!packet.IsValidData())
		return -1;

	if (bufferSize < packet.GetDataSize())
		return 0;

	if (pPacket)
		*pPacket = pBuffer;

	return packet.GetDataSize();
}

int TrustedPacketBufferHandler::ToBuffer(byte* pBuffer, int bufferSize, const byte* pPacket, const byte** pSendBuffer) const
{
	Packet packet(pPacket);
	if (pSendBuffer)
		*pSendBuffer = pPacket;

	return packet.GetDataSize();
}
