#include "PacketBuilder.h"

PacketBuilder::PacketBuilder(Packet::ProtocolType protocol /* = 0 */)
	: BinaryBuilder()
{
	if (protocol > 0) {
		int64 offset = Packet::LengthSize;
		BinaryBuilder::Write(protocol, offset);
	}
}

PacketBuilder::PacketBuilder(byte* pBuffer, int64 bufferSize)
	: BinaryBuilder(false, pBuffer, bufferSize)
{
}

PacketBuilder::~PacketBuilder()
{
}

bool PacketBuilder::Append(const PacketBuilder& packetBuilder)
{
	return BinaryBuilder::Append(packetBuilder.GetBuffer(), packetBuilder.GetBufferSize());
}

const Packet::PacketSizeType PacketBuilder::GetSize()
{
	Packet::PacketSizeType size = 0;

	if (GetBufferSize() > Packet::LengthSize && BinaryBuilder::Read(size, 0))
	{

	}

	return size;
}

void PacketBuilder::SetSize(const Packet::PacketSizeType size)
{
	UpdateSize(size);
}

void PacketBuilder::UpdateSize(const Packet::PacketSizeType size /* = 0 */)
{
	const Packet::PacketSizeType bufferSize = (size > 0) ? size : static_cast<Packet::PacketSizeType>(GetBufferSize());
	if (!DEBUG_ASSERT_EXPR_RETURN(bufferSize <= GetMaxBufferSize()))
		return;

	if (bufferSize > 0) {
		if (!BinaryBuilder::Write(bufferSize, 0))
		{
		}
	}
}