#pragma once
#include "VulkanTools.h"
#include "VulkanAllocator.h"

namespace VkLibrary {

	struct ImageInfo
	{
		VkImage Image = nullptr;
		VkImageView ImageView = nullptr;
		VkSampler Sampler = nullptr;
		VmaAllocation MemoryAllocation = nullptr;
	};

	struct ImageSpecification
	{
		uint8_t* Data = nullptr;
		uint32_t Width;
		uint32_t Height;
		VkFormat Format;
		uint32_t LayerCount = 1;
		VkImageUsageFlags Usage;
		VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT;

		std::string DebugName = "Image";
	};

	// TODO: Add option to enable or disable the use of a staging buffer
	// TODO: Check a resource release queue or intrusive refrence counting system before releasing image
	// TODO: Change size according to image format

	class Image
	{
	public:
		Image(ImageSpecification specification);
		~Image();

	public:
		void Release();
		void Resize(uint32_t width, uint32_t height);

		inline const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Init();

	private:
		ImageInfo m_ImageInfo;
		VkDescriptorImageInfo m_DescriptorImageInfo;
		uint32_t m_Size = 0;

		ImageSpecification m_Specification;
	};

}