#include "pch.h"
#include "Material.h"
#include "Core/Application.h"
#include <glm/gtc/type_ptr.hpp>

namespace VkLibrary {

	static Image* s_DefaultCubeMap;
	static Image* s_DefaultTexture;

	Material::Material(Ref<Shader> shader)
		:	m_Shader(shader)
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

		const auto& bufferDescriptions = m_Shader->GetShaderBufferDescriptions();
		const auto& resourceDescriptions = m_Shader->GetShaderResourceDescriptions();
		const auto& pushConstantRanges = m_Shader->GetPushConstantRanges();
		const auto& writeDescriptorSets = m_Shader->GetWriteDescriptorSets();

		if (writeDescriptorSets.find(3) == writeDescriptorSets.end())
			return;

		if (s_DefaultCubeMap == nullptr)
		{
			ImageSpecification spec = {};
			spec.Data = new uint8_t[sizeof(uint32_t)];
			spec.Width = 1;
			spec.Height = 1;
			spec.Format = ImageFormat::RGBA8;
			spec.Usage = ImageUsage::STORAGE_IMAGE_CUBE;
			spec.DebugName = "Default Cubemap Texture";
			s_DefaultCubeMap = new Image(spec);
		}
		
		if (s_DefaultTexture == nullptr)
		{
			ImageSpecification spec = {};
			spec.Data = new uint8_t[sizeof(uint32_t)];
			spec.Width = 1;
			spec.Height = 1;
			spec.Format = ImageFormat::RGBA8;
			spec.Usage = ImageUsage::STORAGE_IMAGE_2D;
			spec.DebugName = "Default 2D Texture";
			s_DefaultTexture = new Image(spec);
		}


		int UniformBufferCount = 0;
		int StorageBuffersCount = 0;
		if (bufferDescriptions.find(3) != bufferDescriptions.end())
		{
			for (const auto& [binding, bufferDescriptions] : bufferDescriptions.at(3))
			{
				if (bufferDescriptions.Type == ShaderDescriptorType::UNIFORM_BUFFER)
					UniformBufferCount++;
				else if (bufferDescriptions.Type == ShaderDescriptorType::STORAGE_BUFFER)
					StorageBuffersCount++;

				m_WriteDescriptorSets[bufferDescriptions.Name] = m_Shader->FindWriteDescriptorSet(bufferDescriptions.Name);
			}
		}

		int SampledImagesCount = 0;
		int StorageImagesCount = 0;
		if (resourceDescriptions.find(3) != resourceDescriptions.end())
		{
			for (const auto& [binding, resourceDescriptions] : resourceDescriptions.at(3))
			{
				if (resourceDescriptions.Type == ShaderDescriptorType::TEXTURE_2D || resourceDescriptions.Type == ShaderDescriptorType::TEXTURE_CUBE)
					SampledImagesCount++;
				else if (resourceDescriptions.Type == ShaderDescriptorType::STORAGE_IMAGE_2D || resourceDescriptions.Type == ShaderDescriptorType::STORAGE_IMAGE_CUBE)
					StorageImagesCount++;

				m_WriteDescriptorSets[resourceDescriptions.Name] = m_Shader->FindWriteDescriptorSet(resourceDescriptions.Name);

				if (resourceDescriptions.Type == ShaderDescriptorType::TEXTURE_2D || resourceDescriptions.Type == ShaderDescriptorType::STORAGE_IMAGE_2D)
				{
					m_WriteDescriptorSets[resourceDescriptions.Name].pImageInfo = &s_DefaultTexture->GetDescriptorImageInfo();
				}
				else if (resourceDescriptions.Type == ShaderDescriptorType::TEXTURE_CUBE || resourceDescriptions.Type == ShaderDescriptorType::STORAGE_IMAGE_CUBE)
				{
					m_WriteDescriptorSets[resourceDescriptions.Name].pImageInfo = &s_DefaultCubeMap->GetDescriptorImageInfo();
				}
			}
		}

		std::vector<VkDescriptorPoolSize> poolSizes;
		
		if (UniformBufferCount > 0)
		{
			VkDescriptorPoolSize size = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, UniformBufferCount };
			poolSizes.push_back(size);
		}
		if (StorageBuffersCount > 0)
		{
			VkDescriptorPoolSize size = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, StorageBuffersCount };
			poolSizes.push_back(size);
		}
		if (SampledImagesCount > 0)
		{
			VkDescriptorPoolSize size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SampledImagesCount };
			poolSizes.push_back(size);
		}
		if (StorageImagesCount > 0)
		{
			VkDescriptorPoolSize size = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, StorageImagesCount };
			poolSizes.push_back(size);
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = 1;
		descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

		VK_CHECK_RESULT(vkCreateDescriptorPool(device->GetLogicalDevice(), &descriptorPoolCreateInfo, nullptr, &m_Pool));

		m_DescriptorSet = m_Shader->AllocateDescriptorSet(m_Pool, 3);

		for (auto& [name, writeDescriptorSet] : m_WriteDescriptorSets)
		{
			writeDescriptorSet.dstSet = m_DescriptorSet;
		}

		for (const auto& pushConstant : pushConstantRanges)
		{
			if (pushConstant.Name == "u_MaterialData")
			{
				m_PushConstantRangeDescription = pushConstant;
				break;
			}
		}

		if (m_PushConstantRangeDescription.Size != -1)
		{
			m_Buffer = (uint8_t*)malloc(m_PushConstantRangeDescription.Size);
			memset(m_Buffer, 0.0f, m_PushConstantRangeDescription.Size);
		}
	}

	Material::~Material()
	{
	}

	void Material::UpdateDescriptorSet()
	{
		std::vector<VkWriteDescriptorSet> writeDescriptors;
		writeDescriptors.reserve(m_WriteDescriptorSets.size());

		for (auto& [name, writeDescriptorSet] : m_WriteDescriptorSets)
		{
			writeDescriptors.push_back(writeDescriptorSet);
		}

		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		vkUpdateDescriptorSets(device->GetLogicalDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, NULL);
	}

	void Material::SetTexture(const std::string& name, Ref<Image>& image)
	{
		if (image->GetSpecification().Usage == ImageUsage::STORAGE_IMAGE_2D || image->GetSpecification().Usage == ImageUsage::TEXTURE_2D)
		{
			m_WriteDescriptorSets.at(name).pImageInfo = &image->GetDescriptorImageInfo();
		}
		else if (image->GetSpecification().Usage == ImageUsage::STORAGE_IMAGE_CUBE || image->GetSpecification().Usage == ImageUsage::TEXTURE_CUBE)
		{
			m_WriteDescriptorSets.at(name).pImageInfo = &image->GetDescriptorImageInfo();
		}
	}

}