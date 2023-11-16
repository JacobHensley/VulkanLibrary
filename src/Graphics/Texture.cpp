#include "pch.h"
#include "Texture.h"
#include "ComputePipeline.h"
#include "Core/Application.h"
#include <stb/stb_image.h>
#include <nvtt/nvtt.h>
#include <nvtt/nvtt_wrapper.h>

namespace VkLibrary {

	struct MyOutputHandler : public nvtt::OutputHandler
	{
		MyOutputHandler(uint8_t* buffer) : m_Buffer(buffer) { }
		virtual ~MyOutputHandler() { }

		virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) { }
		virtual void endImage() { }

		virtual bool writeData(const void* data, int size)
		{
			memcpy(m_Buffer, data, size);
			return true;
		}

		uint8_t* m_Buffer;
	};

	Texture2D::Texture2D(Texture2DSpecification specification)
		: m_Specification(specification)
	{
		m_Path = m_Specification.path;

		// Load image from disk
		int width, height, bpp;
		stbi_set_flip_vertically_on_load(false);

		if (m_Specification.compress)
		{
			nvtt::Surface image;
			bool load = image.load(m_Specification.path.string().c_str());
			width = image.width();
			height = image.height();

			LOG_DEBUG(load);

			nvtt::Context context(true);

			nvtt::CompressionOptions compressionOptions;
			compressionOptions.setFormat(nvtt::Format_BC7);

			int size = context.estimateSize(image, 1, compressionOptions);
			m_Buffer = (uint8_t*)malloc(size);

			MyOutputHandler outputHandler(m_Buffer);
			nvtt::OutputOptions outputOptions;
			outputOptions.setOutputHandler(&outputHandler);

			context.compress(image, 0, 0, compressionOptions, outputOptions);

			// Create image
			ImageSpecification imageSpecification = {};
			imageSpecification.Data = m_Buffer;
			imageSpecification.Width = width;
			imageSpecification.Height = height;
			imageSpecification.Size = size;
			imageSpecification.Format = ImageFormat::BC7_SRGB;
			imageSpecification.Usage = ImageUsage::TEXTURE_2D;
			imageSpecification.DebugName = (m_Specification.DebugName + ", Image").c_str();

			m_Image = CreateRef<Image>(imageSpecification);
		}
		else
		{
			uint8_t* data = stbi_load(m_Specification.path.string().c_str(), &width, &height, &bpp, 4);
			ASSERT(data, "Failed to load image");

			// Create image
			ImageSpecification imageSpecification = {};
			imageSpecification.Data = data;
			imageSpecification.Width = width;
			imageSpecification.Height = height;
			imageSpecification.Format = specification.sRGB ? ImageFormat::SRGBA8 : ImageFormat::RGBA8;
			imageSpecification.Usage = ImageUsage::TEXTURE_2D;
			imageSpecification.DebugName = (m_Specification.DebugName + ", Image").c_str();

			m_Image = CreateRef<Image>(imageSpecification);

			// Free CPU memory
			stbi_image_free(data);
		}
	}

	Texture2D::~Texture2D()
	{
	}

	TextureCube::TextureCube(TextureCubeSpecification specification)
		: m_Specification(specification)
	{
		m_Path = m_Specification.path;

		m_DescriptorPool = VkTools::CreateDescriptorPool();

		ImageSpecification imageSpec;
		imageSpec.Width = 2048;
		imageSpec.Height = 2048;
		imageSpec.Format = ImageFormat::RGBA32F;
		imageSpec.Usage = ImageUsage::STORAGE_IMAGE_CUBE;
		m_Image = CreateRef<Image>(imageSpec);

		Texture2DSpecification textureSpec;
		textureSpec.path = specification.path;
		textureSpec.sRGB = true;
		Ref<Texture2D> equirectangularInput = CreateRef<Texture2D>(textureSpec);

		ComputePipelineSpecification computeSpec;
		computeSpec.Shader = CreateRef<Shader>("assets/shaders/EquirectangularToCubeMap.glsl");
		Ref<ComputePipeline> equiToCubeMapPipeline = CreateRef<ComputePipeline>(computeSpec);
		
		VkDescriptorSet computeDescriptorSet = computeSpec.Shader->AllocateDescriptorSet(m_DescriptorPool, 0);

		std::array<VkWriteDescriptorSet, 2> writeDescriptors;

		writeDescriptors[0] = computeSpec.Shader->FindWriteDescriptorSet("o_CubeMap");
		writeDescriptors[0].dstSet = computeDescriptorSet;
		writeDescriptors[0].pImageInfo = &m_Image->GetDescriptorImageInfo();
		
		writeDescriptors[1] = computeSpec.Shader->FindWriteDescriptorSet("u_EquirectangularTex");
		writeDescriptors[1].dstSet = computeDescriptorSet;
		writeDescriptors[1].pImageInfo = &equirectangularInput->GetDescriptorImageInfo();
		
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

		vkUpdateDescriptorSets(device->GetLogicalDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		
		VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, equiToCubeMapPipeline->GetPipeline());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, equiToCubeMapPipeline->GetPipelineLayout(), 0, 1, &computeDescriptorSet, 0, nullptr);
		
		vkCmdDispatch(commandBuffer, m_Image->GetSpecification().Width / 32, m_Image->GetSpecification().Height / 32, 6);
		
		device->FlushCommandBuffer(commandBuffer, true);
	}

	TextureCube::~TextureCube()
	{
	}

}