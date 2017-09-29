#pragma once

#include "Define.h"

class StringUtil
{
private:
	StringUtil() {}

public:
	static inline int UnicodeToChar(char* dest, const wchar* src, int len)
	{
		return WideCharToMultiByte(CP_ACP, 0, src, -1, dest, len, NULL, NULL);
	}

	static inline int CharToUnicode(wchar* dest, const char* src, int len)
	{
		return MultiByteToWideChar(CP_ACP, 0, src, -1, dest, len);
	}

	static inline int Compare(const char* dest, const char* src, bool ignorecase = false)
	{
		return ignorecase ? _stricmp(src, dest) : strcmp(src, dest);
	}

	static inline int Compare(const wchar* dest, const wchar* src, bool ignorecase = false)
	{
		return ignorecase ? _wcsicmp(dest, src) : wcscmp(dest, src);
	}

	static inline void Copy(char* dest, const char* src, size_t len)
	{
		strcpy_s(dest, len, src);
	}

	static inline void Copy(wchar* dest, const wchar* src, size_t len)
	{
		wcscpy_s(dest, len, src);
	}

	static inline void NCopy(char* dest, size_t destLen, const char* src, size_t srcLen)
	{
		strncpy_s(dest, destLen, src, srcLen);
	}

	static inline void NCopy(wchar* dest, size_t destLen, const wchar* src, size_t srcLen)
	{
		wcsncpy_s(dest, destLen, src, srcLen);
	}

	static inline size_t Length(const char* str)
	{
		return (str == nullptr) ? 0 : strlen(str);
	}

	static inline size_t Length(const wchar* str)
	{
		return (str == nullptr) ? 0 : wcslen(str);
	}

	static inline int Lower(char* str, size_t len)
	{
		return _strlwr_s(str, len);
	}

	static inline int Lower(wchar* str, size_t len)
	{
		return _wcslwr_s(str, len);
	}

	static inline int Upper(char* str, size_t len)
	{
		return _strupr_s(str, len);
	}

	static inline int Upper(wchar* str, size_t len)
	{
		return _wcsupr_s(str, len);
	}

	static bool Format(wchar* str, size_t len, const wchar* fmt, ...);
	static bool Format(char* str, size_t len, const char* fmt, ...);

	static bool Append(wchar* str, size_t len, const wchar* fmt, ...);
	static bool Append(char* str, size_t len, const char* fmt, ...);
};

