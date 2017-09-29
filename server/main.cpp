#pragma once

#include "Define.h"
//#include "BinaryBuilder.h"
#include "PacketBuilder.h"
#include "SystemPacket.h"

void TestPacket();
void TestBinaryBuilder();

int main(int argc, wchar_t *argv[])
{
	TestPacket();
	//TestBinaryBuilder();

	return 0;
}

void TestPacket()
{
	unsigned char buffer[Packet::MaxBufferSize] = { 0, };
	PacketBuilder packetBuilder;

	ResLoginPacket sPacket(1);
	sPacket.PcList << NetLobbyPc(L"riverstyx1", L"快客1", 1231, 567001);
	sPacket.PcList << NetLobbyPc(L"riverstyx2", L"快客2", 1232, 567002);
	sPacket.IdList << 1004;
	sPacket.IdList << 11004;
	sPacket.Serialize(buffer, sizeof(buffer));
	//sPacket.Serialize(packetBuilder);




	ResLoginPacket dPacket;
	dPacket.Deserialize(buffer, sizeof(buffer));
	//dPacket.Deserialize(packetBuilder);

	//int32 version[] = { 1, 2 };
	//byte macAddress[] = { 0x1C, 0x1B, 0x0D, 0x77, 0xA9, 0xB7 };
	//ReqHelloPacket sPacket(1, 0, 36898124338941099, version, macAddress);
	//sPacket.Serialize(buffer, sizeof(buffer));


	//ReqHelloPacket dPacket;
	//dPacket.Deserialize(buffer, sizeof(buffer));
}

void TestBinaryBuilder()
{
	BinaryBuilder binBuilder;

	bool b = true;
	int vs[] = { 1, 2, 3 };

	//binBuilder << b;
	//binBuilder << (int32)4;
	//binBuilder << (int16)2;
	//binBuilder << (int8)1;
	//binBuilder << (int64)8;
	//binBuilder << (float)1.2;
	//binBuilder << (double)3.4;
	binBuilder << b << (int32)4 << (int16)2 << (int8)1 << (int64)8 << (float)1.2 << (double)3.4;
	for (int i = 0; i < _countof(vs); ++i)
	{
		binBuilder << vs[i];
	}
	binBuilder << L"test";
	binBuilder << NetLobbyPc(L"riverstyx1", L"快客1", 1231, 567001);



	bool bb = false;
	int32 n4 = 0;
	int16 n2 = 0;
	int8 n1 = 0;
	int64 n8 = 0;
	float f = 0.0f;
	double d = 0.0f;
	int ns[] = { 0, 0, 0 };
	wchar msg[512] = { 0, };
	NetLobbyPc netPc;

	//binBuilder >> bb;
	//binBuilder >> n4;
	//binBuilder >> n2;
	//binBuilder >> n1;
	//binBuilder >> n8;
	//binBuilder >> f;
	//binBuilder >> d;
	binBuilder >> bb >> n4 >> n2 >> n1 >> n8 >> f >> d;
	for (int i = 0; i < _countof(vs); ++i)
	{
		binBuilder >> ns[i];
	}
	binBuilder >> msg;
	binBuilder >> netPc;
}