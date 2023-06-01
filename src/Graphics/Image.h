#pragma once
#include "VulkanTools.h"
#include "VulkanAllocator.h"

namespace VkLibrary {

	enum class ImageUsage
	{
		NONE = -1, FRAMEBUFFER_ATTACHMENT, TEXTURE_2D, TEXTURE_CUBE, STORAGE_IMAGE_2D, STORAGE_IMAGE_CUBE
	};

	enum class ImageFormat
	{
		NONE = -1, RGBA8, RGBA32F, DEPTH24_STENCIL8
	};

	struct ImageInfo
	{
		VkImage Image = VK_NULL_HANDLE;
		VkImageView ImageView = VK_NULL_HANDLE;
		VkSampler Sampler = VK_NULL_HANDLE;
		VmaAllocation MemoryAllocation = VK_NULL_HANDLE;
	};

	struct ImageSpecification
	{
		uint8_t* Data = nullptr;
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t LayerCount = 1;
		ImageFormat Format = ImageFormat::NONE;
		ImageUsage Usage = ImageUsage::NONE;
		std::string DebugName = "Image";
	};

	// TODO: Add support for resource deletion queue in Release()

	class Image
	{
	public:
		Image(ImageSpecification specification);
		~Image();

	public:
		void Release();
		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		static uint32_t GetImageFormatSize(ImageFormat format);
		static VkFormat ImageFormatToVulkan(ImageFormat format);
		
		inline const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Init();

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_Size = 0;

		ImageInfo m_ImageInfo;
		VkDescriptorImageInfo m_DescriptorImageInfo;

		ImageSpecification m_Specification;
	};

}