#pragma once

#include "Define.h"
#include <crtdbg.h>

static	inline void DEBUG_ASSERT_EXPR(bool expr, const wchar* msg = nullptr)
{
	if (!expr) {
		if (msg == nullptr)
			_ASSERT(expr);
		else
			_ASSERT_EXPR(expr, msg);
	}
}

static	inline bool DEBUG_ASSERT_EXPR_RETURN(bool expr, const wchar* msg = nullptr)
{
	if (!expr) {
		if (msg == nullptr)
			_ASSERT(expr);
		else
			_ASSERT_EXPR(expr, msg);
	}

	return expr;
}