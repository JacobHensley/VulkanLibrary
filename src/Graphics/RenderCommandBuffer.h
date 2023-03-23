#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace VkLibrary {

	class RenderCommandBuffer
	{
	public:
		RenderCommandBuffer(uint32_t count = 1);
		~RenderCommandBuffer();

		VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffers[m_CurrentIndex]; }
		
		void Begin();
		void End();
		void Submit();

	private:
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<VkFence> m_Fences;

		uint32_t m_CurrentIndex = 0;
	};

}