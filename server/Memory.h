#pragma once

#include "Define.h"

#ifdef _DEBUG

#define _LEAKCHECK_
#ifdef _LEAKCHECK_
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

static class MemoryLeakCheck
{
public:
	MemoryLeakCheck() {
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // | _CRTDBG_DELAY_FREE_MEM_DF
	}
	~MemoryLeakCheck() {
		_ASSERT(_CrtCheckMemory());
	}
} MemoryLeak;


inline	void*	__cdecl	operator	new(size_t size, const char * file, int line)	throw()
{
	return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
}

inline	void	__cdecl	operator	delete(void* ptr, const char * file, int line)	throw() { DebugBreak(); }

#ifdef malloc
#undef malloc
#endif
#define malloc(s)	(_malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__))
#define new new(__FILE__, __LINE__)

#endif // _LEAKCHECK_

#endif // _DEBUG


#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline	void*	__cdecl operator	new(size_t size, void* buf)		throw() { return buf; }
inline	void	__cdecl operator	delete(void* ptr, void* buf)	throw() { DebugBreak(); }
#endif

#ifndef __PLACEMENT_VEC_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE
inline	void*	__cdecl operator	new[](size_t size, void* buf)	throw() {	return buf;					}
inline	void	__cdecl operator	delete[](void* ptr, void* buf)	throw() {	DebugBreak();				}
#endif

#define alloc_d(type, cnt)			(type*)malloc(sizeof(type) * cnt)
#define realloc_d(type, ptr, cnt)	(type*)realloc(ptr, sizeof(type) * cnt)
#define free_d(ptr)					free(ptr)

inline	size_t	GetAllocatedSize(size_t size) { return ((size + (MEMORY_ALLOCATION_ALIGNMENT - 1)) / MEMORY_ALLOCATION_ALIGNMENT) * MEMORY_ALLOCATION_ALIGNMENT; }
#define GET_ALLOCATED_SIZE(size)		((size + (MEMORY_ALLOCATION_ALIGNMENT - 1)) / MEMORY_ALLOCATION_ALIGNMENT) * MEMORY_ALLOCATION_ALIGNMENT;

