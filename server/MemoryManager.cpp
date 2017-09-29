#include "MemoryManager.h"

//HeapMemoryManager
HeapMemoryManager::HeapMemoryManager()
	: _hHeap(INVALID_HANDLE_VALUE)
{
}

HeapMemoryManager::~HeapMemoryManager()
{
	Finalize();
}

bool HeapMemoryManager::Initialize()
{
	_hHeap = HeapCreate(0, 0, 0);
	if (INVALID_HANDLE_VALUE == _hHeap) {
		return false;
	}

	if (!HeapSetInformation(_hHeap, HeapCompatibilityInformation, ((ULONG*)&LFH), sizeof(LFH))) {
		return false;
	}

	return true;
}

void HeapMemoryManager::Finalize()
{
	if (INVALID_HANDLE_VALUE != _hHeap) {
		HeapDestroy(_hHeap);
		_hHeap = INVALID_HANDLE_VALUE;
	}
}

void* HeapMemoryManager::Alloc(size_t size)
{
	return HeapAlloc(_hHeap, 0, size);
}

void* HeapMemoryManager::AllocZero(size_t size)
{
	return HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, size);
}

void* HeapMemoryManager::Realloc(void* p, size_t size)
{
	return HeapReAlloc(_hHeap, 0, p, size);
}

void* HeapMemoryManager::ReallocZero(void* p, size_t size)
{
	return HeapReAlloc(_hHeap, HEAP_ZERO_MEMORY, p, size);
}

void HeapMemoryManager::Free(void* p)
{
	HeapFree(_hHeap, 0, p);
}

void HeapMemoryManager::FreeZero(void* p)
{
	HeapFree(_hHeap, HEAP_ZERO_MEMORY, p);
}


//StaticPageMemoryManager
StaticPageMemoryManager::StaticPageMemoryManager()
	: _pStart(NULL), _pEnd(NULL), _pCurrent(NULL)
	, _pMemoryManager(NULL)
{

}

StaticPageMemoryManager::~StaticPageMemoryManager()
{

}

bool StaticPageMemoryManager::Initialize(void* page, size_t size, size_t allocated, IMemoryManager* pMemoryManager)
{
	if (!page || size <= 0)
		return false;

	allocated = GetAllocatedSize(allocated);

	_pStart = (char*)page;
	_pEnd = _pStart + size;
	_pCurrent = _pStart + allocated;
	_pMemoryManager = pMemoryManager;

	return true;
}

void StaticPageMemoryManager::Finalize()
{

}

void* StaticPageMemoryManager::Alloc(size_t size)
{
	size_t allocated = GetAllocatedSize(size);

	if (allocated > FreeSize())
	{
		if (_pMemoryManager)
			return _pMemoryManager->Alloc(allocated);
		else
			return NULL;
	}
	_pCurrent += allocated;

	return (_pCurrent - allocated);
}

void StaticPageMemoryManager::Free(void* p)
{
	if (_pStart > p || _pEnd <= p)
	{
		if (_pMemoryManager)
			_pMemoryManager->Free(p);
	}
}




//PagePoolMemoryManager
PagePoolMemoryManager::PagePoolMemoryManager()
	: _allocationGranularity(0), _pageSize(0), _allocPageSize(0)
	, _pBlockStart(0), _pBlockEnd(0)
{

}

PagePoolMemoryManager::~PagePoolMemoryManager()
{
	Finalize();
}

bool PagePoolMemoryManager::Initialize(size_t size)
{
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		_allocationGranularity = systemInfo.dwAllocationGranularity;
		_pageSize = systemInfo.dwPageSize;
	}

	if (_pageSize > size || _allocationGranularity <= size)
		return false;

	for (size_t currentPageSize = _pageSize; currentPageSize < _allocationGranularity; currentPageSize *= 2)
	{
		if (size == currentPageSize)
			break;
		if (size < currentPageSize)
			return false;
	}
	_allocPageSize = size;

	return true;
}

void PagePoolMemoryManager::Finalize()
{
	_lock.Enter();

	for each(char* block in _blockList)
	{
		VirtualFree(block, _allocationGranularity, MEM_RESERVE);
		VirtualFree(block, 0, MEM_RESERVE);
	}
	_blockList.clear();

	_lock.Leave();
}

void* PagePoolMemoryManager::Alloc()
{
	ulong allocSize = 0;
	void* p = nullptr;

	_lock.Enter();
	if (_pBlockEnd <= _pBlockStart)
	{
		_pBlockStart = (char*)VirtualAlloc(NULL, _allocationGranularity, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		_pBlockEnd = _pBlockStart + _allocationGranularity;

		_blockList.push_back(_pBlockStart);
	}
	p = _pBlockStart;
	_pBlockStart += _allocPageSize;
	_lock.Leave();

	return p;
}