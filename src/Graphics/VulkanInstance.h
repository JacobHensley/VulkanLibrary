#pragma once
#include "Core/Core.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	class VulkanInstance
	{
	public:
		VulkanInstance(const std::string& name);
		~VulkanInstance();

	public:
		inline VkInstance GetInstanceHandle() const { return m_Instance; }

	private:
		void Init();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		std::string m_Name;
	};

}