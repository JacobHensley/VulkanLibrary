#pragma once
#include "Shader.h"
#include "VertexBufferLayout.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct GraphicsPipelineSpecification
	{
		Ref<Shader> Shader = nullptr;
		Ref<VertexBufferLayout> Layout = nullptr;
		VkRenderPass TargetRenderPass = VK_NULL_HANDLE;
		bool DepthWrite = true;
		bool Blend = true;
	};

	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(GraphicsPipelineSpecification specification);
		~GraphicsPipeline();

	public:
		inline VkPipeline GetPipeline() const { return m_Pipeline; }
		inline VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
	
	private:
		void Init();

	private:
		GraphicsPipelineSpecification m_Specification;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};
}