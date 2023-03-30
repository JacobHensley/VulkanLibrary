#pragma once
#include "Shader.h"
#include "Texture2D.h"
#include "glm/glm.hpp"

namespace VkLibrary {

	// TODO: Find a better way to manage descriptor pools
	// TODO: Add support for a uniform buffer filled with an arbitrary data layout set by the shader

	class Material
	{
	public:
		Material(Ref<Shader> shader);
		~Material();

	public:
		void SetTexture(const std::string& name, Ref<Texture2D> texture);
		void UpdateDescriptorSet();

		VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }

	private:
		VkDescriptorPool CreateDescriptorPool();

	private:
		Ref<Shader> m_Shader;

		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		std::vector<VkWriteDescriptorSet> m_WriteDescriptors;
	};

}