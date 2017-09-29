#include "Packet.h"
#include "BinaryUtil.h"

Packet::Packet(const byte* pData)
	: _pData(pData)
{
	_pDataSize = GetSize();
}

Packet::~Packet()
{

}

Packet::PacketSizeType Packet::GetSize(const byte* pBuffer)
{
	if (!DEBUG_ASSERT_EXPR_RETURN(pBuffer != nullptr))
		return false;

	Packet::PacketSizeType dataSize = 0;

	size_t size = BinaryUtil::Read(pBuffer, MaxBufferSize, 0, dataSize);
	if (size == BinaryUtil::BufferOverflow) {
		DEBUG_ASSERT_EXPR(false);
		return 0;
	}
	else if (size == 0) {
		DEBUG_ASSERT_EXPR(false);
		return 0;
	}

	return dataSize;
}

bool Packet::IsValidData()
{
	if (MaxBufferSize < _pDataSize || 0 >= _pDataSize)
		return false;

	return true;
}

Packet::PacketSizeType Packet::GetSize()
{
	if (!DEBUG_ASSERT_EXPR_RETURN(_pData != nullptr))
		return false;

	return GetSize(_pData);
}