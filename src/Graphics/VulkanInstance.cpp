#include "pch.h"
#include "VulkanInstance.h"
#include "VulkanTools.h"
#include "VulkanExtensions.h"
#include <GLFW/glfw3.h>

static bool s_EnableValidation = true;
static const std::vector<const char*> s_ValidationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

namespace VkLibrary {

	namespace Utils {

		static bool CheckValidationSupport()
		{
			// Get layer info
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			// Check to ensure all validation layers are in availableLayers
			for (const char* layerName : s_ValidationLayers)
			{
				bool found = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					return false;
				}
			}

			return true;
		}

	}

	VulkanInstance::VulkanInstance(const std::string& name)
		:	m_Name(name)
	{
		Init();
	}

	VulkanInstance::~VulkanInstance()
	{
		VkTools::FreeDebugCallback(m_Instance);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanInstance::Init()
	{
		ASSERT(!(s_EnableValidation && !Utils::CheckValidationSupport()), "Requested validation layers not available");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = m_Name.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = m_Name.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (s_EnableValidation) 
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();

		if (s_EnableValidation)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = s_ValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_Instance));

		LoadInstanceExtensions(m_Instance);
		VkTools::SetupDebugging(m_Instance);
	}

}