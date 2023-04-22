#include "pch.h"
#include "Image.h"
#include "VulkanBuffers.h"
#include "Core/Application.h"

namespace VkLibrary {

	Image::Image(ImageSpecification specification)
		:	m_Specification(specification)
	{
		Resize(m_Specification.Width, m_Specification.Height);
	}

	Image::~Image()
	{
		Release();
	}

	void Image::Init()
	{
		Ref<VulkanDevice> device = Application::GetVulkanDevice();

		m_Size = m_Specification.Width * m_Specification.Height * GetImageFormatSize(m_Specification.Format);
		VkFormat format = ImageFormatToVulkan(m_Specification.Format);
		bool isDepthFormat = VkTools::IsDepthFormat(format);
		bool isCube = m_Specification.Usage == ImageUsage::TEXTURE_CUBE || m_Specification.Usage == ImageUsage::STORAGE_IMAGE_CUBE;
		uint32_t layerCount = isCube ? 6 : m_Specification.LayerCount;
		VkImageAspectFlags aspectFlag = isDepthFormat ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = layerCount;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { m_Width, m_Height, 1 };
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

		if (m_Specification.Usage == ImageUsage::FRAMEBUFFER_ATTACHMENT)
		{
			imageCreateInfo.usage |= isDepthFormat ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		else if (m_Specification.Usage == ImageUsage::STORAGE_IMAGE_2D || m_Specification.Usage == ImageUsage::STORAGE_IMAGE_CUBE)
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		else if (m_Specification.Usage == ImageUsage::TEXTURE_2D || m_Specification.Usage == ImageUsage::TEXTURE_CUBE)
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		if (isCube)
		{
			imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		// Allcate memory on the GPU
		VulkanAllocator allocator(m_Specification.DebugName);
		m_ImageInfo.MemoryAllocation = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_ImageInfo.Image);
		VkTools::SetImageName(m_ImageInfo.Image, m_Specification.DebugName.c_str());

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if ((m_Specification.Usage == ImageUsage::TEXTURE_2D || m_Specification.Usage == ImageUsage::TEXTURE_CUBE) && m_Specification.Data)
		{
			StagingBuffer stagingBuffer(m_Specification.Data, m_Size, m_Specification.DebugName + ", Staging Buffer");
			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			VkImageSubresourceRange range;
			range.aspectMask = aspectFlag;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = layerCount;

			// Transfer image from undefined layout to transfer destination optimal layout
			VkTools::InsertImageMemoryBarrier(
				commandBuffer,
				m_ImageInfo.Image,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				range);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;
			copyRegion.imageSubresource.aspectMask = aspectFlag;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = layerCount;
			copyRegion.imageExtent = { m_Width, m_Height, 1 };

			// Copy staging buffer to image on the GPU
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.GetBuffer(), m_ImageInfo.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			// Transfer image from transfer destination optimal layout to shader read optimal layout
			VkTools::InsertImageMemoryBarrier(
				commandBuffer,
				m_ImageInfo.Image,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				range);

			device->FlushCommandBuffer(commandBuffer, true);
		}
		else if (m_Specification.Usage == ImageUsage::STORAGE_IMAGE_2D || m_Specification.Usage == ImageUsage::STORAGE_IMAGE_CUBE)
		{
			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			VkImageSubresourceRange range;
			range.aspectMask = aspectFlag;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = layerCount;

			// Transfer image from undefined layout to general layout
			VkTools::InsertImageMemoryBarrier(
				commandBuffer,
				m_ImageInfo.Image,
				0,
				VK_ACCESS_SHADER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				range);

			// Submit and free command buffer
			device->FlushCommandBuffer(commandBuffer, true);

			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else if (m_Specification.Usage == ImageUsage::FRAMEBUFFER_ATTACHMENT)
		{
			m_DescriptorImageInfo.imageLayout = isDepthFormat ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		// Create image view
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = isCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlag;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = layerCount;
		imageViewCreateInfo.image = m_ImageInfo.Image;

		VK_CHECK_RESULT(vkCreateImageView(device->GetLogicalDevice(), &imageViewCreateInfo, nullptr, &m_ImageInfo.ImageView));
		VkTools::SetImageViewName(m_ImageInfo.ImageView, (m_Specification.DebugName + ", Image View").c_str());

		// Create image sampler
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 20.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;

		VK_CHECK_RESULT(vkCreateSampler(device->GetLogicalDevice(), &samplerCreateInfo, nullptr, &m_ImageInfo.Sampler));
		VkTools::SetSamplerName(m_ImageInfo.Sampler, (m_Specification.DebugName + ", Sampler").c_str());

		m_DescriptorImageInfo.imageView = m_ImageInfo.ImageView;
		m_DescriptorImageInfo.sampler = m_ImageInfo.Sampler;
	}

	void Image::Release()
	{
		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

		if (!m_ImageInfo.Image)
			return;

		VulkanAllocator allocator(m_Specification.DebugName);
		allocator.DestroyImage(m_ImageInfo.Image, m_ImageInfo.MemoryAllocation);

		vkDestroyImageView(device, m_ImageInfo.ImageView, nullptr);
		vkDestroySampler(device, m_ImageInfo.Sampler, nullptr);

		m_ImageInfo.Image = nullptr;
		m_ImageInfo.MemoryAllocation = nullptr;
		m_ImageInfo.ImageView = nullptr;
		m_ImageInfo.Sampler = nullptr;
	}

	void Image::Resize(uint32_t width, uint32_t height)
	{
		Release();

		m_Width = width;
		m_Height = height;

		Init();
	}

	uint32_t Image::GetImageFormatSize(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGBA8:		    return 4;
		case ImageFormat::RGBA32F:			return 16;
		case ImageFormat::DEPTH24_STENCIL8: return 4;
		}

		ASSERT(false, "Unknown Type");
		return 0;
	};

	VkFormat Image::ImageFormatToVulkan(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGBA8:		    return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::RGBA32F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ImageFormat::DEPTH24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
		}

		ASSERT(false, "Unknown Type");
		return (VkFormat)0;
	};

}