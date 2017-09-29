#pragma once

#include "Define.h"
#include "Lock.h"
#include <vector>

class IMemoryManager
{
public:
	IMemoryManager() {}
	virtual ~IMemoryManager() {}

	virtual	void*	Alloc(size_t size) = 0;
	virtual	void*	AllocZero(size_t size) = 0;
	virtual	void*	Realloc(void* p, size_t size) = 0;
	virtual	void*	ReallocZero(void* p, size_t size) = 0;
	virtual	void	Free(void* p) = 0;
	virtual	void	FreeZero(void* p) = 0;
};


class HeapMemoryManager : IMemoryManager
{
public:
	static	const ulong LFH = 2;

public:
	HeapMemoryManager();
	~HeapMemoryManager();

	bool	Initialize();
	void	Finalize();

	void*	Alloc(size_t size) override;
	void*	AllocZero(size_t size) override;
	void*	Realloc(void* p, size_t size) override;
	void*	ReallocZero(void* p, size_t size) override;
	void	Free(void* p) override;
	void	FreeZero(void* p) override;

private:
	HANDLE	_hHeap;
};


class StaticPageMemoryManager : IMemoryManager
{
public:
	StaticPageMemoryManager();
	~StaticPageMemoryManager();

	bool	Initialize(void* page, size_t size, size_t allocated, IMemoryManager* pMemoryManager);
	void	Finalize();
	inline	size_t	FreeSize() { return (_pEnd - _pCurrent); }

	void*	Alloc(size_t size) override;
	void*	AllocZero(size_t size) override { return nullptr; }
	void*	Realloc(void* p, size_t size) override { return nullptr; }
	void*	ReallocZero(void* p, size_t size) override { return nullptr; }
	void	Free(void* p) override;
	void	FreeZero(void* p) override { return; }

private:
	char*	_pStart;
	char*	_pEnd;
	char*	_pCurrent;
	IMemoryManager*	_pMemoryManager;
};


class PagePoolMemoryManager
{
public:
	PagePoolMemoryManager();
	~PagePoolMemoryManager();

	bool	Initialize(size_t size);
	void	Finalize();

	void*	Alloc();

private:
	typedef std::vector<char*>	BlockList;

	ulong	_allocationGranularity;
	ulong	_pageSize;
	size_t	_allocPageSize;
	char*	_pBlockStart;
	char*	_pBlockEnd;
	BlockList		_blockList;
	CriticalSection	_lock;
};


class MemoryBufferUtil
{
public:
	MemoryBufferUtil()
		: m_pStart(nullptr), m_pEnd(nullptr), m_pCurrent(nullptr) {}
	~MemoryBufferUtil() {}

	bool	Initialize(void* buf, size_t size, size_t allocated)
	{
		if (!buf || size <= 0)
			return false;

		m_pStart = (char*)buf;
		m_pEnd = m_pStart + size;
		m_pCurrent = m_pStart + allocated;

		return true;
	}
	void	Finalize() {}

	void	Reset() { m_pCurrent = m_pStart; }
	size_t	FreeSize() { return (m_pEnd - m_pCurrent); }
	size_t	UsedSize() { return (m_pCurrent - m_pStart); }
	size_t	BufferSize() { return (m_pEnd - m_pStart); }
	char*	CurrentBuffer() { return m_pCurrent; }

	void	SetUsedBuffer(size_t size)
	{
		m_pCurrent += size;
	}

	void*	GetBuffer(size_t size)
	{
		size_t allocated = size;
		if (allocated > FreeSize())
		{
			return nullptr;
		}
		m_pCurrent += allocated;

		memset(m_pCurrent - allocated, 0, size);

		return (m_pCurrent - allocated);
	}

	template <typename _Type>
	_Type*	GetBuffer(size_t size = sizeof(_Type))
	{
		size_t allocated = size;
		if (allocated > FreeSize())
		{
			return nullptr;
		}

		m_pCurrent += allocated;
		memset(m_pCurrent - allocated, 0, size);

		return (_Type*)(m_pCurrent - allocated);
	}

private:
	char*	m_pStart;
	char*	m_pEnd;
	char*	m_pCurrent;
};
