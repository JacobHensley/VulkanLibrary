#include "pch.h"
#include "RenderCommandBuffer.h"
#include "Core/Application.h"
#include "VulkanTools.h"

namespace VkLibrary {

	RenderCommandBuffer::RenderCommandBuffer(uint32_t count)
		: m_CommandBuffers(count), m_Fences(count)
	{
		Ref<VulkanDevice> device = Application::GetVulkanDevice();
		QueueFamilyIndices queueIndices = device->GetQueueFamilyIndices();

		// Create command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueIndices.Graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(device->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool));

		// Create command buffers
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(device->GetLogicalDevice(), &allocInfo, m_CommandBuffers.data()));

		// Create fence
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (VkFence& fence : m_Fences)
			VK_CHECK_RESULT(vkCreateFence(device->GetLogicalDevice(), &fenceCreateInfo, nullptr, &fence));
	}

	RenderCommandBuffer::~RenderCommandBuffer()
	{
		Ref<VulkanDevice> device = Application::GetVulkanDevice();
		vkDestroyCommandPool(device->GetLogicalDevice(), m_CommandPool, nullptr);

		// Destroy fence
		for (VkFence fence : m_Fences)
			vkDestroyFence(device->GetLogicalDevice(), fence, nullptr);
	}

	void RenderCommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(GetCommandBuffer(), &begin_info);
	}

	void RenderCommandBuffer::End()
	{
		vkEndCommandBuffer(GetCommandBuffer());
	}

	void RenderCommandBuffer::Submit()
	{
		Ref<VulkanDevice> device = Application::GetVulkanDevice();

		VkCommandBuffer commandBuffer = GetCommandBuffer();

		// Submit info
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Submit command buffer and signal fence when it's done
		VK_CHECK_RESULT(vkResetFences(device->GetLogicalDevice(), 1, &m_Fences[m_CurrentIndex]));
		VK_CHECK_RESULT(vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, m_Fences[m_CurrentIndex]));

		m_CurrentIndex = (m_CurrentIndex + 1) % (uint32_t)m_CommandBuffers.size();
		VK_CHECK_RESULT(vkWaitForFences(device->GetLogicalDevice(), 1, &m_Fences[m_CurrentIndex], VK_TRUE, UINT32_MAX));
	}

}