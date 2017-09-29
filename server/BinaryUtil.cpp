#include "BinaryUtil.h"
#include "StringUtil.h"

BinaryUtil::BinaryUtil()
{
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const int8 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint8 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const int16 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint16 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const int32 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint32 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const int64 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint64 value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const bool value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const float value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const double value)
{
	return WriteType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const char* value)
{
	size_t sizeAffected = 0, size = 0;
	size_t length = StringUtil::Length(value);

	size = WriteType<uint16>(pBuffer, bufferSize, offset + sizeAffected, static_cast<uint16>(length));
	if (!DEBUG_ASSERT_EXPR_RETURN(size != BinaryUtil::BufferOverflow)) {
		return BinaryUtil::BufferOverflow;
	}

	sizeAffected += size;

	if (length > 0) {
		size = WriteType<char>(pBuffer, bufferSize, offset + sizeAffected, value, length);
		if (!DEBUG_ASSERT_EXPR_RETURN(size != BinaryUtil::BufferOverflow)) {
			return BinaryUtil::BufferOverflow;
		}

		sizeAffected += size;
	}

	return sizeAffected;
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const wchar* value)
{
	size_t sizeAffected = 0, size = 0;
	size_t length = StringUtil::Length(value);

	size = WriteType<uint16>(pBuffer, bufferSize, offset + sizeAffected, static_cast<uint16>(length));
	if (!DEBUG_ASSERT_EXPR_RETURN(size != BinaryUtil::BufferOverflow)) {
		return BinaryUtil::BufferOverflow;
	}

	sizeAffected += size;

	if (length > 0) {
		size = WriteType<wchar>(pBuffer, bufferSize, offset + sizeAffected, value, length);
		if (!DEBUG_ASSERT_EXPR_RETURN(size != BinaryUtil::BufferOverflow)) {
			return BinaryUtil::BufferOverflow;
		}

		sizeAffected += size;
	}

	return sizeAffected;
}

size_t BinaryUtil::Write(byte* pBuffer, size_t bufferSize, size_t offset, const void* pValue, size_t size)
{
	if (!DEBUG_ASSERT_EXPR_RETURN(pBuffer != nullptr)) return 0;
	if (!DEBUG_ASSERT_EXPR_RETURN(bufferSize > (offset + size))) return 0;
	if (!DEBUG_ASSERT_EXPR_RETURN(size > 0)) return 0;

	memcpy(pBuffer + offset, pValue, size);

	return size;
}




size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, int8& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint8& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, int16& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint16& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, int32& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint32& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, int64& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint64& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, bool& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, float& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, double& value)
{
	return ReadType(pBuffer, bufferSize, offset, value);
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, char* value)
{
	size_t sizeAffected = 0, size = 0;
	uint16 length = 0;

	size = Read(pBuffer, bufferSize, offset, length);
	if (512 < length) {
		return BinaryUtil::BufferOverflow;
	}

	sizeAffected += size;

	if (length > 0) {
		size = length * sizeof(char);

		memcpy(value, pBuffer + offset + sizeAffected, size);
		if (!DEBUG_ASSERT_EXPR_RETURN(length == StringUtil::Length(value))) {
			return BinaryUtil::BufferOverflow;
		}

		sizeAffected += size;
	}

	return sizeAffected;
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, wchar* value)
{
	size_t sizeAffected = 0, size = 0;
	uint16 length = 0;

	size = Read(pBuffer, bufferSize, offset, length);
	if (512 < length) {
		return BinaryUtil::BufferOverflow;
	}

	sizeAffected += size;

	if (length > 0) {
		size = length * sizeof(wchar);

		memcpy(value, pBuffer + offset + sizeAffected, size);
		if (!DEBUG_ASSERT_EXPR_RETURN(length == StringUtil::Length(value))) {
			return BinaryUtil::BufferOverflow;
		}

		sizeAffected += size;
	}

	return sizeAffected;
}

size_t BinaryUtil::Read(const byte* pBuffer, size_t bufferSize, size_t offset, void* pValue, size_t size)
{
	if (pBuffer == nullptr) return 0;
	if (bufferSize < (offset + size)) return BufferOverflow;
	if (size <= 0) return 0;

	memcpy(pValue, pBuffer + offset, size);

	return size;
}
