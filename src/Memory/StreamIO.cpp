#include "pch.h"
#include "StreamIO.h"

namespace VkLibrary {

	void StreamWriter::WriteBuffer(const Buffer buffer, bool writeSize)
	{
		if (writeSize)
			WriteRaw<uint64_t>(buffer.Size);

		WriteData((const char*)buffer.Data, buffer.Size);
	}

	void StreamWriter::WriteString(const std::string& string)
	{
		WriteRaw<uint64_t>(string.size());
		WriteData(string.data(), string.size());
	}

	Buffer VkLibrary::StreamReader::ReadBuffer(uint64_t size, bool readSize)
	{
		uint64_t size_final = size;
		if (readSize)
			size_final = ReadRaw<uint64_t>();

		char* buffer = new char[size_final];
		ReadData(buffer, size_final);

		return Buffer({ buffer, size_final });
	}

	std::string StreamReader::ReadString()
	{
		uint32_t size = ReadRaw<uint32_t>();
		char* buffer = new char[size];
		ReadData(buffer, size);

		std::string str = std::string(buffer, size);

		delete[] buffer;

		return str;
	}
}