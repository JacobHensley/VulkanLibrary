VK_SDK_PATH = os.getenv("VK_SDK_PATH")

include "vendor/GLFW"
include "vendor/SPIRV-Cross"
include "vendor/imgui"

IncludeDir = {}
IncludeDir["VulkanSDK"]         = VK_SDK_PATH .. "/include"
IncludeDir["GLFW"]              = "vendor/GLFW/include"
IncludeDir["glm"]               = "vendor/glm"
IncludeDir["spdlog"]            = "vendor/spdlog/include"
IncludeDir["VMA"]               = "vendor/VMA/include"
IncludeDir["SPIRVCross"]        = "vendor/SPIRV-Cross"
IncludeDir["imgui"]             = "vendor/imgui"
IncludeDir["stb_image"]         = "vendor/stb"
IncludeDir["DXC"] 				= "vendor/DXC/include"

project "VulkanLibrary"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin/intermediates/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files
	{
		-- VulkanLibrary
		"src/**.cpp",
		"src/**.h",
		-- STB
		"vendor/stb/**.cpp",
		"vendor/stb/**.h",
		-- TinyGltf
		"vendor/tinygltf/**.cpp",
		"vendor/tinygltf/**.hpp",
		"vendor/tinygltf/**.h",
	}

	includedirs
	{
		"src",
		"vendor",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.VMA}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.DXC}",
	}

	links 
	{ 
		"GLFW",
		"SPIRV-Cross",
		"imgui",
		"vendor/DXC/lib/dxcompiler.lib",
		VK_SDK_PATH .. "/Lib/vulkan-1.lib",
		VK_SDK_PATH .. "/Lib/shaderc_shared.lib",
	}

	function VulkanLibraryIncludeDirectories(directory)
		includedirs {
			path.join(directory, "src"),
			path.join(directory, "vendor"),
			"%{IncludeDir.VulkanSDK}",
			path.join(directory, "%{IncludeDir.GLFW}"),
			path.join(directory, "%{IncludeDir.glm}"),
			path.join(directory, "%{IncludeDir.spdlog}"),
			path.join(directory, "%{IncludeDir.VMA}"),
			path.join(directory, "%{IncludeDir.SPIRVCross}"),
			path.join(directory, "%{IncludeDir.imgui}"),
			path.join(directory, "%{IncludeDir.stb_image}"),
			path.join(directory, "%{IncludeDir.DXC}"),
		}
	end

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"