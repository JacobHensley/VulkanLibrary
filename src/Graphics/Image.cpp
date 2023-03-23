#include "pch.h"
#include "Image.h"
#include "VulkanBuffers.h"
#include "Core/Application.h"

namespace VkLibrary {

	Image::Image(ImageSpecification specification)
		:	m_Specification(specification)
	{
		Init();
	}

	Image::~Image()
	{
		Release();
	}

	void Image::Init()
	{
		uint32_t size = m_Specification.Width * m_Specification.Height * 4;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = m_Specification.Format;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = m_Specification.LayerCount;
		imageCreateInfo.samples = m_Specification.SampleCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { m_Specification.Width, m_Specification.Height, 1 };
		imageCreateInfo.usage = m_Specification.Usage | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		// Allocate and create image object
		VulkanAllocator allocator(m_Specification.DebugName);
		m_ImageInfo.MemoryAllocation = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_ImageInfo.Image);
		VkTools::SetImageName(m_ImageInfo.Image, m_Specification.DebugName.c_str());

		bool isDepth = VkTools::IsDepthFormat(m_Specification.Format);
		VkImageAspectFlags aspectFlag = isDepth ? (VK_IMAGE_ASPECT_DEPTH_BIT) : VK_IMAGE_ASPECT_COLOR_BIT;

		Ref<VulkanDevice> device = Application::GetVulkanDevice();

		if (m_Specification.Data)
		{
			// Create staging buffer with image data	
			StagingBuffer stagingBuffer(m_Specification.Data, size, m_Specification.DebugName + ", Staging Buffer");

			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			// Range of image to copy (only the first mip)
			VkImageSubresourceRange range;
			range.aspectMask = aspectFlag;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = m_Specification.LayerCount;

			// Transfer image from undefined layout to destination optimal for copying into
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

			// Define what part of the image to copy
			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;
			copyRegion.imageSubresource.aspectMask = aspectFlag;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = m_Specification.LayerCount;
			copyRegion.imageExtent.width = m_Specification.Width;
			copyRegion.imageExtent.height = m_Specification.Height;
			copyRegion.imageExtent.depth = 1;

			// Copy CPU-GPU buffer into GPU-ONLY texture
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.GetBuffer(), m_ImageInfo.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			// Transfer image from destination optimal layout to shader read optimal
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

			// Submit and free command buffer
			device->FlushCommandBuffer(commandBuffer, true);
		}

		// Create image view
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_Specification.Format;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlag;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = m_Specification.LayerCount;
		imageViewCreateInfo.image = m_ImageInfo.Image;

		VK_CHECK_RESULT(vkCreateImageView(device->GetLogicalDevice(), &imageViewCreateInfo, nullptr, &m_ImageInfo.ImageView));
		VkTools::SetImageViewName(m_ImageInfo.ImageView, (m_Specification.DebugName + ", Image View").c_str());

		// Create sampler
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

		// Create descriptor image info
		if (imageCreateInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT)
		{
			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			// Range of image to copy (only the first mip)
			VkImageSubresourceRange range;
			range.aspectMask = aspectFlag;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = m_Specification.LayerCount;

			// Transfer image from destination optimal layout to shader read optimal
			VkTools::InsertImageMemoryBarrier(
				commandBuffer,
				m_ImageInfo.Image,
				0,
				VK_ACCESS_SHADER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				m_DescriptorImageInfo.imageLayout,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				range);

			// Submit and free command buffer
			device->FlushCommandBuffer(commandBuffer, true);
		}
		else
		{
			m_DescriptorImageInfo.imageLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			// Range of image to copy (only the first mip)
			VkImageSubresourceRange range;
			range.aspectMask = aspectFlag;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = m_Specification.LayerCount;

			// Transfer image from UNDEFINED to final layout
			VkTools::InsertImageMemoryBarrier(
				commandBuffer,
				m_ImageInfo.Image,
				0,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				m_DescriptorImageInfo.imageLayout,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				range);

			// Submit and free command buffer
			device->FlushCommandBuffer(commandBuffer, true);
		}

		m_DescriptorImageInfo.imageView = m_ImageInfo.ImageView;
		m_DescriptorImageInfo.sampler = m_ImageInfo.Sampler;
	}

	void Image::Release()
	{
		if (!m_ImageInfo.Image)
			return;

		VulkanAllocator allocator(m_Specification.DebugName);
		allocator.DestroyImage(m_ImageInfo.Image, m_ImageInfo.MemoryAllocation);

		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

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

		m_Specification.Width = width;
		m_Specification.Height = height;

		Init();
	}

}