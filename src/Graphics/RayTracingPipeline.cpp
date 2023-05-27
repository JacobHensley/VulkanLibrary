#include "pch.h"
#include "RayTracingPipeline.h"
#include "Core/Application.h"
#include "Graphics/VulkanTools.h"

namespace VkLibrary {

	namespace Utils {


		inline uint32_t AlignedSize(uint32_t value, uint32_t alignment)
		{
			return (value + alignment - 1) & ~(alignment - 1);
		}

		static VkStridedDeviceAddressRegionKHR GetSbtEntryStridedDeviceAddressRegion(VkBuffer buffer, uint32_t handleCount = 1)
		{
			const auto& props = Application::GetVulkanDevice()->GetRayTracingPipelineProperties();

			uint32_t handleSizeAligned = AlignedSize(props.shaderGroupHandleSize, props.shaderGroupHandleAlignment);
			VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{};
			stridedDeviceAddressRegionKHR.deviceAddress = VulkanAllocator::GetBufferDeviceAddress(buffer);
			stridedDeviceAddressRegionKHR.stride = handleSizeAligned;
			stridedDeviceAddressRegionKHR.size = handleCount * handleSizeAligned;
			return stridedDeviceAddressRegionKHR;
		}

		static RTBufferInfo CreateShaderBindingTable()
		{
			const auto& props = Application::GetVulkanDevice()->GetRayTracingPipelineProperties();
			VulkanAllocator allocator("RayTracingPipeline");

			RTBufferInfo bufferInfo;

			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = props.shaderGroupHandleSize;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			bufferInfo.Memory = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, bufferInfo.Buffer);

			bufferInfo.StridedDeviceAddressRegion = GetSbtEntryStridedDeviceAddressRegion(bufferInfo.Buffer);

			return bufferInfo;
		}
	}


	RayTracingPipeline::RayTracingPipeline(const RayTracingPipelineSpecification& specification)
		:  m_Specification(specification)
	{
		Init();
	}

	RayTracingPipeline::~RayTracingPipeline()
	{
	}

	void RayTracingPipeline::Init()
	{
		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

		const uint32_t MaxStorageBufferDescriptorCount = 2048;

		std::array<VkDescriptorSetLayoutBinding, 10> layoutBindings = {
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 2),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 3),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 4, MaxStorageBufferDescriptorCount),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 5, MaxStorageBufferDescriptorCount),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 6),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 7),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 8),
			VkTools::CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 9, MaxStorageBufferDescriptorCount),
		};

		std::array<VkDescriptorBindingFlags, layoutBindings.size()> bindingFlags =
		{
			0,											// Binding 0: Acceleration Structure
			0,											// Binding 1: Storage Image
			0,											// Binding 2: Storage Image
			0,											// Binding 3: Camera Uniform Buffer
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,	// Binding 4: Vertex Buffers
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,	// Binding 5: Index Buffers
			0,											// Binding 6: Submesh Data
			0,											// Binding 7: Scene Buffer
			0,											// Binding 8: Material Buffer
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,	// Binding 9: Textures
		};

		VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingsCreateInfo{};
		descriptorSetLayoutBindingsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		descriptorSetLayoutBindingsCreateInfo.pBindingFlags = bindingFlags.data();
		descriptorSetLayoutBindingsCreateInfo.bindingCount = (uint32_t)bindingFlags.size();
		
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingsCreateInfo;
		descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();
		descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)layoutBindings.size();
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

		if (m_Specification.RayGenShader)
		{
			const VkPipelineShaderStageCreateInfo& shaderStage = m_Specification.RayGenShader->GetShaderCreateInfo()[0];
			shaderStages.emplace_back(shaderStage);

			VkRayTracingShaderGroupCreateInfoKHR& shaderGroup = shaderGroups.emplace_back();
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = (uint32_t)(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

			m_ShaderBindingTable.emplace_back(Utils::CreateShaderBindingTable());
		}

		if (m_Specification.MissShader)
		{
			const VkPipelineShaderStageCreateInfo& shaderStage = m_Specification.MissShader->GetShaderCreateInfo()[0];
			shaderStages.emplace_back(shaderStage);

			VkRayTracingShaderGroupCreateInfoKHR& shaderGroup = shaderGroups.emplace_back();
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = (uint32_t)(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

			m_ShaderBindingTable.emplace_back(Utils::CreateShaderBindingTable());
		}

		if (m_Specification.ClosestHitShader)
		{
			const VkPipelineShaderStageCreateInfo& shaderStage = m_Specification.ClosestHitShader->GetShaderCreateInfo()[0];
			shaderStages.emplace_back(shaderStage);

			VkRayTracingShaderGroupCreateInfoKHR& shaderGroup = shaderGroups.emplace_back();
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.closestHitShader = (uint32_t)(shaderStages.size()) - 1;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

			m_ShaderBindingTable.emplace_back(Utils::CreateShaderBindingTable());
		}

		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo{};
		rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
		rayTracingPipelineCreateInfo.pStages = shaderStages.data();
		rayTracingPipelineCreateInfo.groupCount = (uint32_t)shaderGroups.size();
		rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
		rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 4;
		rayTracingPipelineCreateInfo.layout = m_PipelineLayout;
		VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, &m_Pipeline));

		const auto& props = Application::GetVulkanDevice()->GetRayTracingPipelineProperties();

		const uint32_t handleSize = props.shaderGroupHandleSize;
		const uint32_t handleSizeAligned = Utils::AlignedSize(props.shaderGroupHandleSize, props.shaderGroupHandleAlignment);
		const uint32_t groupCount = (uint32_t)shaderGroups.size();
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		m_ShaderHandleStorage.resize(sbtSize);
		VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(device, m_Pipeline, 0, groupCount, sbtSize, m_ShaderHandleStorage.data()));

		VulkanAllocator allocator("RayTracingPipeline");
		{
			uint8_t* raygenHandle = allocator.MapMemory<uint8_t>(m_ShaderBindingTable[0].Memory);
			memcpy(raygenHandle, m_ShaderHandleStorage.data(), handleSize);
			allocator.UnmapMemory(m_ShaderBindingTable[0].Memory);
		}
		{
			uint8_t* missHandle = allocator.MapMemory<uint8_t>(m_ShaderBindingTable[1].Memory);
			memcpy(missHandle, m_ShaderHandleStorage.data() + handleSizeAligned, handleSize);
			allocator.UnmapMemory(m_ShaderBindingTable[1].Memory);
		}
		{
			uint8_t* closestHitHandle = allocator.MapMemory<uint8_t>(m_ShaderBindingTable[2].Memory);
			memcpy(closestHitHandle, m_ShaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
			allocator.UnmapMemory(m_ShaderBindingTable[2].Memory);
		}
	}

	

}