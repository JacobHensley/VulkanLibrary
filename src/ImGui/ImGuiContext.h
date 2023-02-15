#pragma once
#include <Vulkan/vulkan.h>

namespace VkLibrary {

	class ImGuiLayer
	{
	public: 
		ImGuiLayer();
		~ImGuiLayer();

		// CPU side
		void BeginFrame();
		void EndFrame();
		
		// Vulkan side
		void BeginRenderPass();
		void EndRenderPass();

		void RenderDrawLists();
	private:
		void Init();

	private:
		VkDescriptorPool m_DescriptorPool;
	};

}