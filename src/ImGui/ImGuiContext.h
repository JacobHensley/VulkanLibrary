#pragma once
#include <Vulkan/vulkan.h>

namespace VkLibrary {

	class ImGuiContext
	{
	public: 
		ImGuiContext();
		~ImGuiContext();

		void BeginFrame();
		void EndFrame();

	private:
		void Init();

	private:
		VkDescriptorPool m_DescriptorPool;
	};

}