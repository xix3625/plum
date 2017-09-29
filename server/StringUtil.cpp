#include "StringUtil.h"
#include "TypeDefine.h"

bool StringUtil::Format(wchar* str, size_t len, const wchar* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int n = _vsnwprintf_s(str, len, _TRUNCATE, fmt, args);
	va_end(args);

	return true;
}

bool StringUtil::Format(char* str, size_t len, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int n = _vsnprintf_s(str, len, _TRUNCATE, fmt, args);
	va_end(args);

	return true;
}

bool StringUtil::Append(wchar* str, size_t len, const wchar* fmt, ...)
{
	wchar src[512] = { 0, };
	va_list args;
	va_start(args, fmt);
	int n = _vsnwprintf_s(src, _countof(src), _TRUNCATE, fmt, args);
	va_end(args);

	size_t strLen = StringUtil::Length(str);
	if (len <= (strLen + n + 2))
		return false;

	wcscat_s(str, len, src);
	return true;
}

bool StringUtil::Append(char* str, size_t len, const char* fmt, ...)
{
	char src[512] = { 0, };
	va_list args;
	va_start(args, fmt);
	int n = _vsnprintf_s(src, _countof(src), _TRUNCATE, fmt, args);
	va_end(args);

	size_t strLen = StringUtil::Length(str);
	if (len <= (strLen + n + 2))
		return false;

	strcat_s(str, len, src);
	return true;
}