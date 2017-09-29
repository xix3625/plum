#include "RefCountObject.h"

RefCountObject::RefCountObject()
{
	AddRef();
}

RefCountObject::~RefCountObject()
{
	if (_count != 0) {
		DEBUG_ASSERT_EXPR(false);
	}
}
