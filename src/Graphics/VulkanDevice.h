#pragma once
#include "pch.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct QueueFamilyIndices
	{
		uint32_t Graphics = UINT32_MAX;
		uint32_t Compute = UINT32_MAX;
		uint32_t Transfer = UINT32_MAX;

		bool IsComplete()
		{
			return Graphics != UINT32_MAX && Compute != UINT32_MAX && Transfer != UINT32_MAX;
		};
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice();
		~VulkanDevice();

	public:
		inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; };
		inline VkDevice GetLogicalDevice() const { return m_LogicalDevice; };
		inline SwapChainSupportDetails GetSwapChainSupportDetails() const { return QuerySwapChainSupport(m_PhysicalDevice); }

		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free);

		inline QueueFamilyIndices GetQueueFamilyIndices() const { return m_QueueFamilyIndices; };
		inline VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }

		const VkPhysicalDeviceProperties2& GetDeviceProperties() const { return m_DeviceProperties; }
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetRayTracingPipelineProperties() const { return m_RayTracingPipelineProperties; }

	private:
		void Init();

		uint32_t IsDeviceSuitable(VkPhysicalDevice device);
		uint32_t GetQueueFamilyIndex(VkQueueFlags queueFlags);
		std::vector<VkDeviceQueueCreateInfo> GetQueueCreateInfo(VkQueueFlags requestedQueueTypes);

		std::vector<std::string> GetSupportedDeviceExtensions(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		QueueFamilyIndices m_QueueFamilyIndices;
		SwapChainSupportDetails m_SwapChainSupportDetails;
		std::vector<std::string> m_SupportedDeviceExtensions;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;

		VkPhysicalDeviceProperties2 m_DeviceProperties{};
		VkPhysicalDeviceFeatures m_DeviceFeatures{};
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		VkPhysicalDeviceAccelerationStructurePropertiesKHR m_AccelerationStructureProperties{};
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingPipelineProperties{};
	};

}