#pragma once

#include "Define.h"
#include "BinaryUtil.h"

class BinaryBuilder
{
public:
	static	const int64 DefaultSize = 0x1000;

public:
	BinaryBuilder();
	BinaryBuilder(bool bExternal, byte* pBuffer, int64 bufferSize);
	virtual ~BinaryBuilder();
	void Clear();
	bool Append(const BinaryBuilder& builder);
	bool Append(const byte* pBuffer, int64 size);

	template<typename Type>
	bool Write(Type& value, int64 offset = -1)
	{
		if (offset > 0 && !DEBUG_ASSERT_EXPR_RETURN(offset < _maxBufferSize)) {
			return false;
		}

		size_t size = BinaryUtil::Write(GetBuffer(), _maxBufferSize, (offset < 0) ? _wPos : offset, value);
		if (size == BinaryUtil::BufferOverflow) {
			DEBUG_ASSERT_EXPR(false);
			return false;
		}
		else if (size == 0) {
			DEBUG_ASSERT_EXPR(false);
			return false;
		}

		if (offset < 0)
			_wPos += size;

		return true;
	}
	bool Write(const char* value, int64 offset = -1);
	bool Write(const wchar* value, int64 offset = -1);

	template<typename Type>
	bool Read(Type& value, int64 offset = -1)
	{
		if (offset > 0 && !DEBUG_ASSERT_EXPR_RETURN(offset < _wPos)) {
			return false;
		}

		size_t size = BinaryUtil::Read(GetBuffer(), _maxBufferSize, (offset < 0) ? _rPos : offset, value);
		if (size == BinaryUtil::BufferOverflow) {
			DEBUG_ASSERT_EXPR(false);
			return false;
		}
		else if (size == 0) {
			DEBUG_ASSERT_EXPR(false);
			return false;
		}

		if (offset < 0)
			_rPos += size;

		return true;
	}
	bool Read(char* value, int64 offset = -1);
	bool Read(wchar* value, int64 offset = -1);

	BinaryBuilder &operator << (const uint8 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const int8 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const uint16 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const int16 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const uint32 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const int32 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const uint64 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const int64 value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const bool value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const float value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const double value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const char* value)		{	Write(value); return *this;	}
	BinaryBuilder &operator << (const wchar* value)		{	Write(value); return *this;	}
	template<typename Type>
	BinaryBuilder &operator << (Type& value)			{	Write(value); return *this;	}

	BinaryBuilder &operator >> (uint8& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (int8& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (uint16& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (int16& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (uint32& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (int32& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (uint64& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (int64& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (bool& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (float& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (double& value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (char* value)			{	Read(value); return *this;	}
	BinaryBuilder &operator >> (wchar* value)			{	Read(value); return *this;	}
	template<typename Type>
	BinaryBuilder &operator >> (Type& value)			{	Read(value); return *this;	}

	inline int64 GetMaxBufferSize() const
	{
		return _maxBufferSize;
	}

	inline byte *GetBuffer()
	{
		return (_pExternalBuffer) ? _pExternalBuffer : _buffer;
	}

	inline const byte *GetBuffer() const
	{
		return (_pExternalBuffer) ? _pExternalBuffer : _buffer;
	}

	inline int64 GetBufferSize()
	{
		return GetWritePosition();
	}

	inline const int64 GetBufferSize() const
	{
		return GetWritePosition();
	}

	inline int64 GetReadPosition() const
	{
		return _rPos;
	}

	inline int64 GetWritePosition() const
	{
		return _wPos;
	}

private:
	const int64		_maxBufferSize;
	byte			_buffer[DefaultSize];
	byte			*_pExternalBuffer;
	int64			_wPos;
	int64			_rPos;
};

