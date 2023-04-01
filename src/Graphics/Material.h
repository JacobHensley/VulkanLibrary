#pragma once
#include "Shader.h"
#include "Texture2D.h"
#include <glm/glm.hpp>

namespace VkLibrary {

	// TODO: Add support for a push constant filled with an arbitrary data layout set by the shader
	// TODO: Move write descriptors to constructer
	// TODO: Add prepare function to update any Descriptors
	// TODO: Generate pool based on number of uniforms
	  
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