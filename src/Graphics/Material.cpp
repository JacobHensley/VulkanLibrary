#include "pch.h"
#include "Material.h"
#include "Core/Application.h"

#define MATERIAL_DESCRIPTOR_SET_ID 3

namespace VkLibrary {

	Material::Material(Ref<Shader> shader)
		:	m_Shader(shader)
	{
		m_DescriptorPool = CreateDescriptorPool();
		m_DescriptorSet = m_Shader->AllocateDescriptorSet(m_DescriptorPool, MATERIAL_DESCRIPTOR_SET_ID);
	}

	Material::~Material()
	{
	}

	void Material::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors.emplace_back(m_Shader->GenerateWriteDescriptor(name));
		writeDescriptor.dstSet = m_DescriptorSet;
		writeDescriptor.pImageInfo = &texture->GetDescriptorImageInfo();
	}

	void Material::UpdateDescriptorSet()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		vkUpdateDescriptorSets(device->GetLogicalDevice(), m_WriteDescriptors.size(), m_WriteDescriptors.data(), 0, NULL);
	}

	VkDescriptorPool Material::CreateDescriptorPool()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = 1000;
		descriptorPoolCreateInfo.poolSizeCount = 1;
		descriptorPoolCreateInfo.pPoolSizes = poolSizes;

		VkDescriptorPool pool;
		VK_CHECK_RESULT(vkCreateDescriptorPool(device->GetLogicalDevice(), &descriptorPoolCreateInfo, nullptr, &pool));

		return pool;
	}

}