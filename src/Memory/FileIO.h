#pragma once
#include <filesystem>
#include <fstream>
#include "StreamIO.h"

namespace VkLibrary {

	class FileWriter : public StreamWriter
	{
	public:
		FileWriter(const std::filesystem::path& path);
		~FileWriter();

		void WriteData(const char* data, uint64_t size);

	private:
		std::filesystem::path m_Path;
		std::ofstream m_Stream;
	};

	class FileReader : public StreamReader
	{
	public:
		FileReader(const std::filesystem::path& path);
		~FileReader();

		char* ReadData(uint64_t size);
		void ReadData(char* output, uint64_t size);

	private:
		std::filesystem::path m_Path;
		std::ifstream m_Stream;
	};
}