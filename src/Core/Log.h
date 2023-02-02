#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace VkLibrary {

	class Log
	{
	public:
		static void Init();

	public:
		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}

// Core log macros
#define LOG_TRACE(...)    VkLibrary::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    VkLibrary::Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     VkLibrary::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     VkLibrary::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    VkLibrary::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) VkLibrary::Log::GetLogger()->critical(__VA_ARGS__)