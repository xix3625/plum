#pragma once

#include "Define.h"

class RefCountObject;

//namespace boost
//{
//	void intrusive_ptr_add_ref(RefCountObject* p);
//	void intrusive_ptr_release(RefCountObject* p);
//};

void intrusive_ptr_add_ref(RefCountObject* p);
void intrusive_ptr_release(RefCountObject* p);

class RefCountObject
{
public:
	RefCountObject();
	virtual ~RefCountObject();

	virtual	void AddRef()
	{
		if (0 >= _count) {
			DEBUG_ASSERT_EXPR(false);
			return;
		}

		AddReferenceCount();
	}

	virtual	void ReleaseRef()
	{
		if (0 >= _count) {
			DEBUG_ASSERT_EXPR(false);
			return;
		}

		if (0 == ReleaseReferenceCount()) {
			Delete();
		}
	}

protected:
	virtual void Delete() = 0;

protected:
	__forceinline long AddReferenceCount()
	{
		return InterlockedIncrement(&_count);
	}

	__forceinline long ReleaseReferenceCount()
	{
		return InterlockedDecrement(&_count);
	}

	//friend void ::boost::intrusive_ptr_add_ref(RefCountObject* p);
	//friend void ::boost::intrusive_ptr_release(RefCountObject* p);
	friend void ::intrusive_ptr_add_ref(RefCountObject* p);
	friend void ::intrusive_ptr_release(RefCountObject* p);

private:
	long _count;
};

//namespace boost
//{
	inline void intrusive_ptr_add_ref(RefCountObject* p)
	{
		p->AddRef();
	}

	inline void intrusive_ptr_release(RefCountObject* p)
	{
		p->ReleaseRef();
	}
//}