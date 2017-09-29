#include "PacketBuffer.h"

//PacketBuffer
PacketBuffer::PacketBuffer(PacketBufferPool* pPool /* = nullptr */)
	: _pPool(pPool), _sendSize(0), _pSendBuffer(nullptr), _pNext(nullptr)
{
	AddRef();

	memset(_pBuffer, 0, sizeof(_pBuffer));
}

PacketBuffer::~PacketBuffer()
{
}

void PacketBuffer::AddRef()
{
	__super::AddRef();
}

void PacketBuffer::ReleaseRef()
{
	__super::ReleaseRef();
}

void PacketBuffer::Delete()
{
	if (_pPool)
		_pPool->Free(this);
	else
		delete this;
}


//PacketBufferPool
PacketBufferPool::PacketBufferPool(void)
	: _slotSize(0), _pSlotList(nullptr), _allocSlot(0), _freeSlot(0)
	, _allocBuffer(0), _allocCount(0)
{
}

PacketBufferPool::~PacketBufferPool(void)
{
	Finalize();

	if (_pSlotList)
	{
		for (int32 index = 0; index < _slotSize; ++index)
		{
			if (_pSlotList[index])
				delete _pSlotList[index];
		}

		free_d(_pSlotList);
		_pSlotList = nullptr;
	}
}

bool PacketBufferPool::Initialize(int32 slotSize)
{
	_slotSize = slotSize;
	_pSlotList = alloc_d(Slot*, _slotSize);
	_allocSlot = -1;
	_freeSlot = 0;
	_allocBuffer = 0;
	_allocCount = 0;

	for (int32 index = 0; index < _slotSize; ++index)
	{
		_pSlotList[index] = new Slot;
	}

	return true;
}

void PacketBufferPool::Finalize()
{
	FreeAll();
}

PacketBuffer* PacketBufferPool::Alloc()
{
	PacketBuffer* pBuffer = nullptr;
	Slot* pSlot = _pSlotList[InterlockedIncrement((volatile ulong*) & _allocSlot) & (_slotSize - 1)];

	pSlot->Enter();
	if (nullptr != (pBuffer = pSlot->GetBuffer()))
	{
		pSlot->SetBuffer(pBuffer->GetNext());

		pBuffer->PacketBuffer::PacketBuffer(this);
		pSlot->Leave();
	}
	else
	{
		pSlot->Leave();
		pBuffer = new PacketBuffer(this);
		InterlockedIncrement((volatile ulong*)&_allocCount);
	}

	InterlockedIncrement((volatile ulong*)&_allocBuffer);
	return pBuffer;
}

void PacketBufferPool::Free(PacketBuffer* pBuffer)
{
	if (pBuffer == nullptr) return;

	InterlockedDecrement((volatile ulong*)&_allocBuffer);
	Slot* pSlot = _pSlotList[InterlockedDecrement((volatile ulong*) & _freeSlot) & (_slotSize - 1)];

	pSlot->Enter();
	pBuffer->~PacketBuffer();
	pBuffer->SetNext(pSlot->GetBuffer());
	pSlot->SetBuffer(pBuffer);
	pSlot->Leave();
}

void PacketBufferPool::FreeAll()
{
	PacketBuffer* pBuffer = nullptr;

	for (int32 index = 0; index < _slotSize; ++index)
	{
		Slot* pSlot = _pSlotList[index];

		pSlot->Enter();
		while (nullptr != (pBuffer = pSlot->GetBuffer()))
		{
			pSlot->SetBuffer(pBuffer->GetNext());
			delete pBuffer;	//pBuffer->Delete(); pure call È¸ÇÇ by riverstyx
		}
		pSlot->Leave();
	}
}