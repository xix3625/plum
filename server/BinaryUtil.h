#pragma once

#include "Define.h"
#include <list>

template<typename Type, const size_t MaxSize = 100>
struct TEntityList
{
public:
	TEntityList()
	{
		Clear();
	}

	inline void Clear()
	{
		memset(_entity, 0, sizeof(_entity));
		_size = 0;
	}

	inline int Size()
	{
		return _size;
	}

	inline bool IsEmpty()
	{
		return (_size == 0);
	}

	inline bool IsFull()
	{
		return (_size == MaxSize);
	}

	inline bool Add(const Type &item)
	{
		if (IsFull()) return false;

		_entity[_size] = item;
		++_size;

		return true;
	}

	inline bool Add(Type &item)
	{
		if (IsFull()) return false;

		_entity[_size] = item;
		++_size;

		return true;
	}

	inline bool Each(Type &item, const int pos)
	{
		if (pos < 0 || pos >= _size) return false;

		item = _entity[pos];

		return true;
	}

	inline bool operator << (const Type &item)
	{
		return Add(item);
	}

	inline bool operator << (Type &item)
	{
		return Add(item);
	}

private:
	Type	_entity[MaxSize];
	int		_size;
};

class BinaryUtil
{
public:
	static const size_t BufferOverflow = UINT64_MAX;

private:
	BinaryUtil();

public:
	/* Write */
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const int8 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint8 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const int16 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint16 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const int32 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint32 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const int64 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const uint64 value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const bool value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const float value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const double value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const char* value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const wchar* value);
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, const void* pValue, size_t size);
	template<typename Type>
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, Type& value)
	{
		return value.Serialize(pBuffer, bufferSize, offset);
	}
	template<typename Type>
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, std::list<Type>& list)
	{
		size_t size = 0; uint16 count = static_cast<uint16>(list.size());

		size += WriteType<uint16>(pBuffer, bufferSize, offset, count);
		if (count > 0)
		{
			for each (auto type in list) {
				size += Write(pBuffer, bufferSize, offset + size, type);
			}
		}

		return size;
	}
	template<typename Type, const int MaxSize>
	static size_t Write(byte* pBuffer, size_t bufferSize, size_t offset, TEntityList<Type, MaxSize>& list)
	{
		size_t size = 0; uint16 count = static_cast<uint16>(list.Size());

		size += WriteType<uint16>(pBuffer, bufferSize, offset, count);
		if (count > 0)
		{
			int index = 0;
			Type type;

			while (list.Each(type, index++)) {
				size += Write(pBuffer, bufferSize, offset + size, type);
			}
		}

		return size;
	}

	template<typename Type>
	static size_t WriteType(byte* pBuffer, size_t bufferSize, size_t offset, const Type value)
	{
		//EndianConvert(value);
		return Write(pBuffer, bufferSize, offset, &value, sizeof(value));
	}
	template<typename Type>
	static size_t WriteType(byte* pBuffer, size_t bufferSize, size_t offset, const Type* pValue, size_t cnt)
	{
		//EndianConvert(value);
		return Write(pBuffer, bufferSize, offset, pValue, cnt * sizeof(Type));
	}




	/* Read */
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, int8& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint8& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, int16& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint16& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, int32& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint32& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, int64& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, uint64& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, bool& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, float& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, double& value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, char* value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, wchar* value);
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, void* pValue, size_t size);
	template<typename Type>
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, Type& value)
	{
		return value.Deserialize(pBuffer, bufferSize, offset);
	}
	template<typename Type>
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, std::list<Type>& list)
	{
		size_t size = 0; uint16 count = 0;

		size += ReadType<uint16>(pBuffer, bufferSize, offset + size, count);
		if (count > 0)
		{
			for (int index = 0; index < count; ++index) {
				Type type;
				size += Read(pBuffer, bufferSize, offset + size, type);
				list.push_back(type);
			}
		}

		return size;
	}
	template<typename Type, const int MaxSize>
	static size_t Read(const byte* pBuffer, size_t bufferSize, size_t offset, TEntityList<Type, MaxSize>& list)
	{
		size_t size = 0; uint16 count = 0;

		size += ReadType<uint16>(pBuffer, bufferSize, offset + size, count);
		if (count > 0)
		{
			for (int index = 0; index < count; ++index) {
				Type type;
				size += Read(pBuffer, bufferSize, offset + size, type);
				list << type;
			}
		}

		return size;
	}

	template <typename Type>
	static size_t ReadType(const byte* pBuffer, size_t bufferSize, size_t offset, Type& value)
	{
		return Read(pBuffer, bufferSize, offset, &value, sizeof(value));
	}
	template<typename Type>
	static size_t ReadType(const byte* pBuffer, size_t bufferSize, size_t offset, Type* pValue, size_t cnt)
	{
		return Read(pBuffer, bufferSize, offset, pValue, cnt * sizeof(Type));
	}
};

