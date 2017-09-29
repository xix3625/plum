#pragma once

#include "Packet.h"
#include "BinaryUtil.h"

enum ProtocolId : uint16
{
	ReqHello = 4097, //(1 << 12 | 1)
	ResHello = 4098, //(1 << 12 | 2)
	ReqLogin, 
	ResLogin,
};



class PacketBuilder;

struct ReqHelloPacket : public ISerializable
{
private:
	ReqHelloPacket(const ReqHelloPacket &);
	ReqHelloPacket& operator =(const ReqHelloPacket &);

public:
	ReqHelloPacket();
	ReqHelloPacket(uint8 szPlatformType, uint8 szMiniType, int64 device, int32 version[2], byte macAddress[6]);
	size_t		Serialize(byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);
	size_t		Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);

public:
	uint16		Protocol;
	uint8		SZPlatformType;
	uint8		SZMiniType;
	int64		Device;
	int32		Version[2];
	byte		MacAddress[6];
};

struct ResHelloPacket : public ISerializable
{
private:
	ResHelloPacket(const ResHelloPacket &);
	ResHelloPacket& operator =(const ResHelloPacket &);

public:
	ResHelloPacket();
	ResHelloPacket(uint32 resultCode);
	size_t		Serialize(byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);
	size_t		Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);

public:
	uint16		Protocol;
	uint32		ResultCode;
};









struct NetLobbyPc : public ISerializable
{
private:
	//NetLobbyPc(const NetLobbyPc &);
	//NetLobbyPc& operator =(const NetLobbyPc &);

public:
	NetLobbyPc();
	NetLobbyPc(wchar id[32], wchar name[32], int64 exp, int64 money);
	size_t		Serialize(byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);
	size_t		Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);

public:
	wchar		Id[32];
	wchar		Name[32];
	int64		Exp;
	int64		Money;
	//TEntityList<NetLobbyPcEquip, 10>	Items;
};

struct ReqLoginPacket : public ISerializable
{
private:
	ReqLoginPacket(const ReqLoginPacket &);
	ReqLoginPacket& operator =(const ReqLoginPacket &);

public:
	ReqLoginPacket();
	ReqLoginPacket(const wchar id[32], const wchar password[32]);
	size_t		Serialize(byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);
	size_t		Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);

public:
	uint16		Protocol;
	wchar		Id[32];
	wchar		Password[32];
};

struct ResLoginPacket : public ISerializable
{
private:
	ResLoginPacket(const ResLoginPacket &);
	ResLoginPacket& operator =(const ResLoginPacket &);

public:
	ResLoginPacket();
	ResLoginPacket(uint16 resultCode);
	bool		Serialize(PacketBuilder &packetBuilder);
	bool		Deserialize(PacketBuilder &packetBuilder);
	size_t		Serialize(byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);
	size_t		Deserialize(const byte *pBuffer, size_t bufferSize, size_t offset = Packet::LengthSize);

public:
	uint16		Protocol;
	uint32		ResultCode;
	TEntityList<NetLobbyPc, 10>	PcList;
	TEntityList<int32, 10>		IdList;
};