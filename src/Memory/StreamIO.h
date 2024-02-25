#pragma once
#include <string>
#include <vector>
#include "Buffer.h"
#include "Core/Log.h"

namespace VkLibrary {

	class StreamWriter
	{
	public:
		virtual ~StreamWriter() = default;

		virtual void WriteData(const char* data, uint64_t size) = 0;

		void WriteBuffer(const Buffer buffer, bool writeSize = true);
		void WriteString(const std::string& buffer);

		template<typename T>
		void WriteRaw(const T& data)
		{
			WriteData((const char*)&data, sizeof(data));
		}

		template<typename T>
		void WriteArray(const std::vector<T>& v)
		{
			WriteRaw(v.size());
			
			if constexpr (std::is_trivial<T>())
			{
				WriteData((const char*)v.data(), v.size() * sizeof(T));
			}
			else
			{
				for (const T& element : v)
				{
					WriteRaw(element);
				}
			}

		}
	};

	class StreamReader
	{
	public:
		virtual ~StreamReader() = default;

		virtual char* ReadData(uint64_t size) = 0;
		virtual void ReadData(char* output, uint64_t size) = 0;

		Buffer ReadBuffer(uint64_t size = 0, bool readSize = true);
		std::string ReadString();

		template<typename T>
		T& ReadRaw()
		{
			char* value = ReadData(sizeof(T));
			return (T&)value;
		}

		template<typename T>
		std::vector<T> ReadArray()
		{
			uint64_t size = ReadRaw<uint64_t>();
			std::vector<T> output(size);
			ReadData((char*)output.data(), output.size() * sizeof(T));
			return output;
		}
	};

}