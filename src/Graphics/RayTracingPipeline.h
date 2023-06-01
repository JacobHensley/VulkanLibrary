#pragma once
#include "Shader.h"
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct RayTracingPipelineSpecification
	{
		Ref<Shader> RayGenShader;
		Ref<Shader> MissShader;
		Ref<Shader> ClosestHitShader;
	};

	struct RTBufferInfo
	{
		VkBuffer Buffer;
		VmaAllocation Memory;
		VkStridedDeviceAddressRegionKHR StridedDeviceAddressRegion;
	};

	// TODO: Add support for resource deletion queue in destructor
	// TODO: Set max number for storage buffers based on hardware

	class RayTracingPipeline
	{
	public:
		RayTracingPipeline(const RayTracingPipelineSpecification& specification);
		~RayTracingPipeline();

	public:
		inline VkPipeline GetPipeline() { return m_Pipeline; }
		inline VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }

		inline const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
		const std::vector<RTBufferInfo>& GetShaderBindingTable() { return m_ShaderBindingTable; }

	private:
		void Init();

	private:
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;

		Ref<Shader> m_Shader;

		std::vector<uint8_t> m_ShaderHandleStorage;
		std::vector<RTBufferInfo> m_ShaderBindingTable;

		RayTracingPipelineSpecification m_Specification;
	};

}