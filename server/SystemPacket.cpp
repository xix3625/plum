#include "SystemPacket.h"
#include "Define.h"
#include "BinaryUtil.h"
#include "StringUtil.h"
#include "PacketBuilder.h"

ReqHelloPacket::ReqHelloPacket()
	: ReqHelloPacket(0, 0, 0L, nullptr, nullptr)
{

}

ReqHelloPacket::ReqHelloPacket(uint8 szPlatformType, uint8 szMiniType, int64 device, int32 version[2], byte macAddress[6])
	: Protocol(ProtocolId::ReqHello), SZPlatformType(szPlatformType), SZMiniType(szMiniType), Device(device)
{
	memset(Version, 0, sizeof(Version));
	memset(MacAddress, 0, sizeof(MacAddress));

	if (version != nullptr)
		memcpy(Version, version, sizeof(Version));
	if (macAddress != nullptr)
		memcpy(MacAddress, macAddress, sizeof(MacAddress));
}

size_t ReqHelloPacket::Serialize(byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	size_t size = 0;
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Protocol);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, SZPlatformType);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, SZMiniType);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Device);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Version, _countof(Version));
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, MacAddress, _countof(MacAddress));
	size += BinaryUtil::Write(pBuffer, bufferSize, 0, static_cast<Packet::PacketSizeType>(offset + size));
	return size;
}

size_t ReqHelloPacket::Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	Packet::PacketSizeType packetSize = 0;
	if (BinaryUtil::Read(pBuffer, bufferSize, 0, packetSize) <= 0) {
		return 0;
	}

	size_t size = 0;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Protocol);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, SZPlatformType);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, SZMiniType);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Device);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Version, _countof(Version));
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, MacAddress, _countof(MacAddress));
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	return size + offset;
}


ResHelloPacket::ResHelloPacket()
	: ResHelloPacket(0)
{

}

ResHelloPacket::ResHelloPacket(uint32 resultCode)
	: Protocol(ResHello), ResultCode(resultCode)
{

}

size_t ResHelloPacket::Serialize(byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	size_t size = 0;
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Protocol);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, ResultCode);
	size += BinaryUtil::Write(pBuffer, bufferSize, 0, static_cast<Packet::PacketSizeType>(offset + size));
	return size;
}

size_t ResHelloPacket::Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	Packet::PacketSizeType packetSize = 0;
	if (BinaryUtil::Read(pBuffer, bufferSize, 0, packetSize) <= 0) {
		return 0;
	}

	size_t size = 0;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Protocol);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, ResultCode);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	return size;
}








NetLobbyPc::NetLobbyPc()
{
	memset(Id, 0, sizeof(Id));
	memset(Name, 0, sizeof(Name));
	Exp = 0L;
	Money = 0L;
}

NetLobbyPc::NetLobbyPc(wchar id[32], wchar name[32], int64 exp, int64 money)
	: Exp(exp), Money(money)
{
	if (StringUtil::Length(id) > 0)
		StringUtil::Copy(Id, id, _countof(Id));
	if (StringUtil::Length(name) > 0)
		StringUtil::Copy(Name, name, _countof(Name));
}

size_t NetLobbyPc::Serialize(byte *pBuffer, size_t bufferSize, size_t offset)
{
	size_t size = 0;
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Id);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Name);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Exp);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Money);
	//size += BinaryUtil::Write(pBuffer, offset + size, Items);
	return size;
}

size_t NetLobbyPc::Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset)
{
	size_t size = 0;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Id);
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Name);
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Exp);
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Money);
	//size += BinaryUtil::Read(pBuffer, offset + size, Items);
	return size;
}




ReqLoginPacket::ReqLoginPacket()
	: ReqLoginPacket(nullptr, nullptr)
{

}

ReqLoginPacket::ReqLoginPacket(const wchar id[32], const wchar password[32])
	: Protocol(ReqLogin)
{
	if (StringUtil::Length(id) > 0)
		StringUtil::Copy(Id, id, _countof(Id));
	if (StringUtil::Length(password) > 0)
		StringUtil::Copy(Password, password, _countof(Password));
}

size_t ReqLoginPacket::Serialize(byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	size_t size = 0;
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Protocol);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Id);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Password);
	size += BinaryUtil::Write(pBuffer, bufferSize, 0, static_cast<Packet::PacketSizeType>(offset + size));
	return size;
}

size_t ReqLoginPacket::Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	Packet::PacketSizeType packetSize = 0;
	if (BinaryUtil::Read(pBuffer, bufferSize, 0, packetSize) <= 0) {
		return 0;
	}

	size_t size = 0;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Protocol);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Id);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Password);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	return size;
}

ResLoginPacket::ResLoginPacket()
	: ResLoginPacket(0)
{

}

ResLoginPacket::ResLoginPacket(uint16 resultCode)
	: Protocol(ResLogin), ResultCode(resultCode)
{

}

bool ResLoginPacket::Serialize(PacketBuilder &packetBuilder)
{
	const Packet::PacketSizeType packetSize = 0;

	if (!packetBuilder.Write(packetSize)) return false;
	if (!packetBuilder.Write(Protocol)) return false;
	if (!packetBuilder.Write(ResultCode)) return false;
	if (!packetBuilder.Write(PcList)) return false;
	if (!packetBuilder.Write(IdList)) return false;
	packetBuilder.UpdateSize();

	return true;
}

bool ResLoginPacket::Deserialize(PacketBuilder &packetBuilder)
{
	Packet::PacketSizeType packetSize = 0;

	if (!packetBuilder.Read(packetSize)) return false;
	if (!packetBuilder.Read(Protocol)) return false;
	if (!packetBuilder.Read(ResultCode)) return false;
	if (!packetBuilder.Read(PcList)) return false;
	if (!packetBuilder.Read(IdList)) return false;

	return true;
}

size_t ResLoginPacket::Serialize(byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	size_t size = 0;
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, Protocol);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, ResultCode);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, PcList);
	size += BinaryUtil::Write(pBuffer, bufferSize, offset + size, IdList);
	size += BinaryUtil::Write(pBuffer, bufferSize, 0, static_cast<Packet::PacketSizeType>(offset + size));
	return size;
}

size_t ResLoginPacket::Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset /* = Packet::LengthSize */)
{
	Packet::PacketSizeType packetSize = 0;
	if (BinaryUtil::Read(pBuffer, bufferSize, 0, packetSize) <= 0) {
		return 0;
	}

	size_t size = 0;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, Protocol);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, ResultCode);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, PcList);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	size += BinaryUtil::Read(pBuffer, bufferSize, offset + size, IdList);
	if (!DEBUG_ASSERT_EXPR_RETURN(packetSize >= size + offset)) return size;
	return size;
}