#include "pch.h"
#include "ComputePipeline.h"
#include "VulkanTools.h"
#include "Core/Application.h"

namespace VkLibrary {

    ComputePipeline::ComputePipeline(ComputePipelineSpecification specification)
        : m_Specification(specification)
    {
        Init();
    }

    ComputePipeline::~ComputePipeline()
    {
        VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

        vkDestroyPipeline(device, m_Pipeline, nullptr);
        if (m_OwnLayout)
            vkDestroyPipelineLayout(device, m_Specification.PipelineLayout, nullptr);
    }

    void ComputePipeline::Init()
    {
		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

		if (!m_Specification.PipelineLayout)
		{
			// Set push constant ranges
			const std::vector<PushConstantRangeDescription>& pushConstantRangeDescriptions = m_Specification.Shader->GetPushConstantRanges();

			std::vector<VkPushConstantRange> pushConstantRanges;
			pushConstantRanges.reserve(pushConstantRangeDescriptions.size());
			for (const auto& pushConstangeRange : pushConstantRangeDescriptions)
			{
				VkPushConstantRange& pcr = pushConstantRanges.emplace_back();
				pcr.stageFlags = pushConstangeRange.ShaderStage;
				pcr.offset = pushConstangeRange.Offset;
				pcr.size = pushConstangeRange.Size;
			}

			const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = m_Specification.Shader->GetDescriptorSetLayouts();

			// Set pipeline layout
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
			pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
			pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_Specification.PipelineLayout));
			m_OwnLayout = true;
		}

		ASSERT(m_Specification.Shader->GetShaderCreateInfo().size() == 1, "Compute pipeline must only have one stage");

		// Create compute pipeline
		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_Specification.PipelineLayout;
		computePipelineCreateInfo.stage = m_Specification.Shader->GetShaderCreateInfo()[0];
		VK_CHECK_RESULT(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_Pipeline));
    }

}