#pragma once
#include <stdint.h>

struct Buffer
{
	void* Data = nullptr;
	uint64_t Size = 0;

	Buffer() = default;

	Buffer(void* data, uint64_t size)
		: Data(data), Size(size)
	{
	}

	void Allocate(uint64_t size)
	{
		Data = malloc(size);
		Size = size;
	}

	void Release()
	{
		free(Data);
		Size = 0;
	}

	template<typename T>
	T* As()
	{
		return (T*)Data;
	}

	operator bool() const
	{
		return (bool)Data;
	}
};