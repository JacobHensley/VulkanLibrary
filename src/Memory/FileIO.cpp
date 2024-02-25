#include "pch.h"
#include "FileIO.h"

namespace VkLibrary 
{
	FileWriter::FileWriter(const std::filesystem::path& path)
		:	m_Path(path)
	{
		m_Stream = std::ofstream(path, std::ofstream::out | std::ofstream::binary);
	}

	FileWriter::~FileWriter()
	{
		m_Stream.close();
	}

	void FileWriter::WriteData(const char* data, uint64_t size)
	{
		m_Stream.write(data, size);
	}

	FileReader::FileReader(const std::filesystem::path& path)
		: m_Path(path)
	{
		m_Stream = std::ifstream(path, std::ifstream::in | std::ifstream::binary);
	}

	FileReader::~FileReader()
	{
		m_Stream.close();
	}

	char* FileReader::ReadData(uint64_t size)
	{
		char* value;
		m_Stream.read(reinterpret_cast<char*>(&value), size);
		return value;
	}

	void FileReader::ReadData(char* output, uint64_t size)
	{
		m_Stream.read(output, size);
	}
}