#include "BinaryBuilder.h"
#include "StringUtil.h"

BinaryBuilder::BinaryBuilder()
	: _maxBufferSize(DefaultSize)
{
	Clear();
}

BinaryBuilder::BinaryBuilder(bool bExternal, byte* pBuffer, int64 bufferSize)
	: BinaryBuilder()
{
	DEBUG_ASSERT_EXPR(pBuffer != nullptr);
	DEBUG_ASSERT_EXPR(bufferSize > 0 && bufferSize <= _maxBufferSize);

	if (bExternal)
		_pExternalBuffer = pBuffer;
	else
		Append(pBuffer, bufferSize);

	_wPos = bufferSize;
}

BinaryBuilder::~BinaryBuilder()
{
}

void BinaryBuilder::Clear()
{
	memset(_buffer, 0, sizeof(_buffer));
	_pExternalBuffer = nullptr;
	_wPos = _rPos = 0;
}

bool BinaryBuilder::Append(const BinaryBuilder& builder)
{
	return Append(builder.GetBuffer(), builder.GetBufferSize());
}

bool BinaryBuilder::Append(const byte* pBuffer, int64 bufferSize)
{
	if (GetBufferSize() + bufferSize > _maxBufferSize) {
		DEBUG_ASSERT_EXPR(false);

		return false;
	}

	size_t size = BinaryUtil::Write(GetBuffer(), _maxBufferSize, _wPos, pBuffer, bufferSize);
	if (size == BinaryUtil::BufferOverflow) {
		DEBUG_ASSERT_EXPR(false);
		return false;
	}
	else if (size == 0) {
		DEBUG_ASSERT_EXPR(false);
		return false;
	}

	_wPos += size;

	return true;
}

bool BinaryBuilder::Write(const char* value, int64 offset /* = -1 */)
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

bool BinaryBuilder::Write(const wchar* value, int64 offset /* = -1 */)
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




bool BinaryBuilder::Read(char* value, int64 offset /* = -1 */)
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

bool BinaryBuilder::Read(wchar* value, int64 offset /* = -1 */)
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
