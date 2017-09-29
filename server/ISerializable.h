#pragma once

#include "TypeDefine.h"

class ISerializable
{
public:
	virtual	size_t	Serialize(byte* pBuffer, size_t bufferSize, size_t offset) = 0;
	virtual	size_t	Deserialize(const byte* pBuffer, size_t bufferSize, size_t offset) = 0;
};
