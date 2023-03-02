#include "pch.h"
#include "TextureCube.h"

#include "Core/Application.h"
#include "VulkanTools.h"

namespace VkLibrary {

	TextureCube::TextureCube(TextureCubeSpecification specification)
		: m_Specification(specification)
	{
		m_Width = m_Specification.Width;
		m_Height = m_Specification.Height;
		m_Format = m_Specification.Format;

		Invalidate();
	}

	TextureCube::~TextureCube()
	{

	}

	void TextureCube::Invalidate()
	{
		VulkanAllocator allocator("TextureCube");
		Ref<VulkanDevice> device = Application::GetVulkanDevice();

		uint32_t mipCount = 1;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = m_Format;
		imageCreateInfo.mipLevels = mipCount;
		imageCreateInfo.arrayLayers = 6;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.extent = { m_Width, m_Height, 1 };
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		m_MemoryAllocation = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Image/*, &m_GPUAllocationSize*/);

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkCommandBuffer layoutCmd = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipCount;
		subresourceRange.layerCount = 6;

		VkTools::SetImageLayout(
			layoutCmd, m_Image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			m_DescriptorImageInfo.imageLayout,
			subresourceRange);

		device->FlushCommandBuffer(layoutCmd, true);

		// Create a texture sampler
		VkSamplerCreateInfo sampler{};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.maxAnisotropy = 1.0f;
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.mipLodBias = 0.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		// Set max level-of-detail to mip level count of the texture
		sampler.maxLod = (float)mipCount;
		// Enable anisotropic filtering
		// This feature is optional, so we must check if it's supported on the device

		sampler.maxAnisotropy = 1.0;
		sampler.anisotropyEnable = VK_FALSE;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(device->GetLogicalDevice(), &sampler, nullptr, &m_DescriptorImageInfo.sampler));

		// Create image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image views containing additional
		// information and sub resource ranges
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		view.format = m_Format;
		view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
		// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 6;
		view.subresourceRange.levelCount = mipCount;
		view.image = m_Image;
		VK_CHECK_RESULT(vkCreateImageView(device->GetLogicalDevice(), &view, nullptr, &m_DescriptorImageInfo.imageView));
	}

}