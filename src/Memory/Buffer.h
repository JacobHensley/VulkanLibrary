#pragma once
#include <stdint.h>

struct Buffer
{
	void* Data = nullptr;
	uint64_t Size = 0;
};