#pragma once

#include "VulkanBuffers.h"
#include "VulkanAllocator.h"

namespace VkLibrary {

	struct TextureCubeSpecification
	{
		uint32_t Width;
		uint32_t Height;
		VkFormat Format;

		std::string DebugName = "TextureCube";
	};

	class TextureCube
	{
	public:
		TextureCube(TextureCubeSpecification specification);
		~TextureCube();

		inline const TextureCubeSpecification& GetSpecification() const { return m_Specification; }
		inline const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }

	private:
		void Invalidate();

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		VkImage m_Image = VK_NULL_HANDLE;
		VkFormat m_Format;
		VkDescriptorImageInfo m_DescriptorImageInfo = {};

		VmaAllocation m_MemoryAllocation = VK_NULL_HANDLE;

		TextureCubeSpecification m_Specification;
	};

}