#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "VulkanExtensions.h"
#include "Core/Application.h"
 
namespace VkLibrary {

	VulkanDevice::VulkanDevice()
	{
		Init();
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);

		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	void VulkanDevice::Init()
	{
		VkInstance vulkanInstance = Application::GetVulkanInstance()->GetInstanceHandle();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
		ASSERT(deviceCount > 0, "Failed to find GPU with Vulkan support")

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

		uint32_t highestDeviceRating = 0;
		for (VkPhysicalDevice device : devices)
		{
			uint32_t rating = IsDeviceSuitable(device);
			if (rating > highestDeviceRating)
			{
				m_PhysicalDevice = device;
				highestDeviceRating = rating;
			}
				
		}

		ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "Could not find suitable device");

		m_SupportedDeviceExtensions = GetSupportedDeviceExtensions(m_PhysicalDevice);
		m_SwapChainSupportDetails = QuerySwapChainSupport(m_PhysicalDevice);

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		m_QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = GetQueueCreateInfo(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		// Set queue priority to 1.0f
		float queuePriority = 1.0f;
		for (auto& queueCreateInfo : queueCreateInfos)
		{
			queueCreateInfo.pQueuePriorities = &queuePriority;
		}

		// Enable device extensions features
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// Enable device extensions here
		std::vector<const char*> deviceExtensions;

		// Required extensions
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);

		VkPhysicalDeviceVulkan12Features v12Features{};
		v12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		v12Features.shaderBufferInt64Atomics = true;
		v12Features.bufferDeviceAddress = true;
		v12Features.descriptorBindingPartiallyBound = true;
		v12Features.descriptorIndexing = true;
		v12Features.runtimeDescriptorArray = true;
		v12Features.bufferDeviceAddress = true;

		// Ray tracing features
		VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{};
		robustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		robustness2Features.nullDescriptor = VK_TRUE;

		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
		accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		accelerationStructureFeatures.pNext = &robustness2Features;
		accelerationStructureFeatures.accelerationStructure = VK_TRUE;
		accelerationStructureFeatures.accelerationStructureCaptureReplay = VK_FALSE;
		accelerationStructureFeatures.accelerationStructureIndirectBuild = VK_FALSE;
		accelerationStructureFeatures.accelerationStructureHostCommands = VK_FALSE;
		accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
		rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rayTracingPipelineFeatures.pNext = &accelerationStructureFeatures;
		rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
		rayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay = VK_FALSE;
		rayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE;
		rayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect = VK_TRUE;
		rayTracingPipelineFeatures.rayTraversalPrimitiveCulling = VK_TRUE;

		v12Features.pNext = &rayTracingPipelineFeatures;

		// Ray tracing extensions
		deviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &v12Features;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		// Check if all enabled device extensions are supported
		if (deviceExtensions.size() > 0)
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				ASSERT(std::find(m_SupportedDeviceExtensions.begin(), m_SupportedDeviceExtensions.end(), enabledExtension) != m_SupportedDeviceExtensions.end(), "Enabled device extension is not present at device level")
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice));

		LoadDeviceExtensions(m_LogicalDevice);

		vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);

		// Create command pool
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
		VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &m_CommandPool));
	}

	uint32_t VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		m_AccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		m_RayTracingPipelineProperties.pNext = &m_AccelerationStructureProperties;

		m_DeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_DeviceProperties.pNext = &m_RayTracingPipelineProperties;

		vkGetPhysicalDeviceProperties2(device, &m_DeviceProperties);
		vkGetPhysicalDeviceFeatures(device, &m_DeviceFeatures);

		if (m_DeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return 100;
		else
			return 10;
	}

	uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlags queueFlags)
	{
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++)
			{
				if ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++)
			{
				if ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++)
		{
			if ((m_QueueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				return i;
			}
		}

		ASSERT(false, "Could not find a matching queue family index")
		return 0;
	}

	std::vector<VkDeviceQueueCreateInfo> VulkanDevice::GetQueueCreateInfo(VkQueueFlags requestedQueueTypes)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlap depending on the implementation

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			m_QueueFamilyIndices.Graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
			queueInfo.queueCount = 1;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			m_QueueFamilyIndices.Graphics = 0;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			m_QueueFamilyIndices.Compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (m_QueueFamilyIndices.Compute != m_QueueFamilyIndices.Graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Compute;
				queueInfo.queueCount = 1;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			m_QueueFamilyIndices.Compute = m_QueueFamilyIndices.Compute;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			m_QueueFamilyIndices.Transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Graphics) && (m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Compute))
			{
				// If transfer family index differs, we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Transfer;
				queueInfo.queueCount = 1;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			m_QueueFamilyIndices.Transfer = m_QueueFamilyIndices.Graphics;
		}

		return queueCreateInfos;
	}

	std::vector<std::string> VulkanDevice::GetSupportedDeviceExtensions(VkPhysicalDevice device)
	{
		std::vector<std::string> supportedExtensions;
		// Get list of supported extensions
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		if (extensionCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extensionCount);
			if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (auto extension : extensions)
				{
					supportedExtensions.push_back(extension.extensionName);
				}
			}
		}

		return supportedExtensions;
	}

	SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice device) const
	{
		SwapChainSupportDetails details;

		VkSurfaceKHR surface = Application::GetWindow()->GetVulkanSurface();
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) 
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) 
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

	VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		// Create command buffer
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = level;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer));

		// Begin command buffer if specified
		if (begin)
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo{};
			commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		}

		return commandBuffer;
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
	{
		ASSERT(commandBuffer != VK_NULL_HANDLE, "Command buffer is invalid");

		// End command buffers
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		// Submit info
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Create fence
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence));

		// Submit command buffer and signal fence when it's done
		VK_CHECK_RESULT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence));
		VK_CHECK_RESULT(vkWaitForFences(m_LogicalDevice, 1, &fence, VK_TRUE, UINT32_MAX));

		// Destroy fence
		vkDestroyFence(m_LogicalDevice, fence, nullptr);

		// Free command buffer if specified
		if (free)
		{
			vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
		}
	}

}