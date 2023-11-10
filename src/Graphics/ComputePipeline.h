#pragma once
#include <vulkan/vulkan.h>
#include "Shader.h"

namespace VkLibrary {

	struct ComputePipelineSpecification
	{
		Ref<Shader> Shader;
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	};

	class ComputePipeline
	{
	public:
		ComputePipeline(ComputePipelineSpecification specification);
		~ComputePipeline();

	public:
		inline VkPipeline GetPipeline() { return m_Pipeline; }
		inline VkPipelineLayout GetPipelineLayout() { return m_Specification.PipelineLayout; }
		
		inline Ref<Shader> GetShader() const { return m_Specification.Shader; }

		inline const ComputePipelineSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Init();

	private:
		ComputePipelineSpecification m_Specification;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		bool m_OwnLayout = false;
	};

}