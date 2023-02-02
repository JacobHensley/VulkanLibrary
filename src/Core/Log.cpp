#include "pch.h"
#include "Log.h"

namespace VkLibrary {

	std::shared_ptr<spdlog::logger> Log::s_Logger;;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T][%l] %v%$");
		spdlog::set_level(spdlog::level::trace);

		s_Logger = spdlog::stderr_color_mt("Logger");
	}

}