#pragma once

#include "Define.h"
#include "RefCountObject.h"
#include "Packet.h"
#include "Lock.h"

class PacketBufferPool;

class PacketBuffer : public RefCountObject
{
public:
	static	const Packet::PacketSizeType	HeaderSize = 0;	// 암호화 정보
	static	const Packet::PacketSizeType	DataSize = Packet::MaxBufferSize;
	static	const Packet::PacketSizeType	MaxBuffer = HeaderSize + DataSize;

public:
	PacketBuffer(PacketBufferPool* pPool = nullptr);
	virtual ~PacketBuffer();

	inline	byte*					GetPacketBuffer() { return _pBuffer + HeaderSize; }
	inline	Packet::PacketSizeType	GetPacketSize() { return Packet::GetSize(GetPacketBuffer()); }
	inline	const byte*				GetSendBuffer() { return _pSendBuffer ? _pSendBuffer : GetPacketBuffer(); }
	inline	int						GetSendSize() { return _pSendBuffer ? _sendSize : GetPacketSize(); }
	inline	void					SetBuffer(const byte* pBuffer, const int size) { memcpy(GetPacketBuffer(), pBuffer, size); }

	inline	void					SetNext(PacketBuffer* pNext) { _pNext = pNext; }
	inline	PacketBuffer*			GetNext() { return _pNext; }
	inline	bool					IsReady() const { return _pSendBuffer ? true : false; }
	inline	void					SetSendBuffer(const byte* pBuffer, int size) {
		_pSendBuffer = pBuffer;
		_sendSize = size;
	}

	void	AddRef() override;
	void	ReleaseRef() override;

protected:
	void	Delete() override;

private:
	PacketBufferPool*	_pPool;
	byte				_pBuffer[MaxBuffer];
	int					_sendSize;
	const byte*			_pSendBuffer;
	PacketBuffer*		_pNext;
};


class PacketBufferPool
{
public:
	static	const int32	DefaultSlotSize	= 16;

private:
	class Slot {
	public:
		Slot() : _pBuffer(nullptr) {}
		inline	void	Enter() { _lock.Enter(); }
		inline	void	Leave() { _lock.Leave(); }
		inline	PacketBuffer*	GetBuffer() { return _pBuffer; }
		inline	void	SetBuffer(PacketBuffer* pBuffer) { _pBuffer = pBuffer; }

	private:
		PacketBuffer*	_pBuffer;
		CriticalSection	_lock;
	};

public:
	PacketBufferPool();
	~PacketBufferPool();
	bool	Initialize(int32 slotSize);
	void	Finalize();

	PacketBuffer*	Alloc();
	void	Free(PacketBuffer* pBuffer);
	void	FreeAll();

private:
	int32	_slotSize;
	Slot**	_pSlotList;
	int32	_allocSlot;
	int32	_freeSlot;
	int32	_allocBuffer;
	int32	_allocCount;
};
