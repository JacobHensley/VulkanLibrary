#pragma once
#include "Shader.h"
#include "Texture.h"
#include <glm/glm.hpp>

namespace VkLibrary {

	// TODO: Add prepare function to update descriptors
	
	class Material
	{
	public:
		Material(Ref<Shader> shader);
		~Material();

	public:
		void UpdateDescriptorSet();

		void SetTexture(const std::string& name, Ref<Image>& image);

		template<typename T>
		void Set(const std::string& name, const T& data)
		{
			for (const auto& member : m_PushConstantRangeDescription.Members)
			{
				if (member.Name == name)
				{
					memcpy(m_Buffer + member.Offset - 64, &data, member.Size);
					return;
				}
			}

			return;
		}

		inline const VkDescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; }
		inline const PushConstantRangeDescription& GetPushConstantRangeDescription() const { return m_PushConstantRangeDescription; }
		inline const uint8_t* GetBuffer() const { return m_Buffer; }

	private:
		Ref<Shader> m_Shader;
		uint8_t* m_Buffer;

		VkDescriptorPool m_Pool = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
		std::unordered_map<std::string, VkWriteDescriptorSet> m_WriteDescriptorSets;
		PushConstantRangeDescription m_PushConstantRangeDescription;
	};

}