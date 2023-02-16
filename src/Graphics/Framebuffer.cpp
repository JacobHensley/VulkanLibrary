#include "pch.h"
#include "Framebuffer.h"
#include "Core/Application.h"

namespace VkLibrary {

	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		:	m_Specification(specification)
	{
		InitAttachmentImages();
		Resize(m_Specification.Width, m_Specification.Height);
	}

	Framebuffer::~Framebuffer()
	{
		Ref<VulkanDevice> device = Application::GetVulkanDevice();

		vkDestroyRenderPass(device->GetLogicalDevice(), m_RenderPass, nullptr);
		vkDestroyFramebuffer(device->GetLogicalDevice(), m_Framebuffer, nullptr);
	}

	void Framebuffer::InitAttachmentImages()
	{
		for (int i = 0; i < m_Specification.AttachmentFormats.size(); i++)
		{
			// If attachment is a depth format use specfic attachment otherwise create a new one in the list
			bool isDepth = VkTools::IsDepthFormat(m_Specification.AttachmentFormats[i]);
			FramebufferAttachment& attachment = isDepth ? m_DepthAttachment : m_ColorAttachments.emplace_back();

			// Create image to fit attachment
			ImageSpecification imageSpecification = {};
			imageSpecification.Data = nullptr;
			imageSpecification.Width = m_Specification.Width;
			imageSpecification.Height = m_Specification.Height;
			imageSpecification.Format = m_Specification.AttachmentFormats[i];
			imageSpecification.Usage = isDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			imageSpecification.SampleCount = VK_SAMPLE_COUNT_1_BIT;
			imageSpecification.DebugName = fmt::format("{}, {} Attachment {}", m_Specification.DebugName, isDepth ? "Depth" : "Color", i);
			attachment.Image = CreateRef<Image>(imageSpecification);

			// Fill attachment description
			attachment.Description = {};
			attachment.Description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.Description.loadOp = m_Specification.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.Description.format = m_Specification.AttachmentFormats[i];
			attachment.Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// Final layout
			if (isDepth || VkTools::IsStencilFormat(m_Specification.AttachmentFormats[i]))
			{
				attachment.Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			else
			{
				attachment.Description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				m_ColorAttachmentCount++;
			}
		}
	}

	bool Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		width *= m_Specification.Scale;
		height *= m_Specification.Scale;

		if (m_Width == width && m_Height == height)
			return false;	

		m_Width = width;
		m_Height = height;

		CreateFramebuffer();

		return true;
	}

	void Framebuffer::CreateFramebuffer()
	{
		// Collect attachment descriptions
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		for (auto& attachment : m_ColorAttachments)
		{
			attachmentDescriptions.push_back(attachment.Description);
		};

		if (m_DepthAttachment)
			attachmentDescriptions.push_back(m_DepthAttachment.Description);

		// Collect attachment references
		std::vector<VkAttachmentReference> colorAttachmentReferences;
		VkAttachmentReference depthAttachmentReference = {};

		bool hasColorAttachment = false;
		bool hasDepthAttachment = false;

		for (uint32_t i = 0; i < m_ColorAttachments.size(); i++)
		{
			FramebufferAttachment attachment = m_ColorAttachments[i];
			colorAttachmentReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			hasColorAttachment = true;
		}

		if (m_DepthAttachment)
		{
			depthAttachmentReference.attachment = (uint32_t)m_ColorAttachments.size();
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			hasDepthAttachment = true;
		}

		// Create suppass using attachment references
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		if (hasColorAttachment)
		{
			subpass.pColorAttachments = colorAttachmentReferences.data();
			subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
		}
		if (hasDepthAttachment)
		{
			subpass.pDepthStencilAttachment = &depthAttachmentReference;
		}

		// Create subpass dependencies
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		Ref<VulkanDevice> device = Application::GetVulkanDevice();

		// Create render pass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass));
		VkTools::SetRenderPassName(m_RenderPass, (m_Specification.DebugName + " RenderPass").c_str());

		// Collect attachment image views
		std::vector<VkImageView> attachmentViews;
		for (auto attachment : m_ColorAttachments)
		{
			attachmentViews.push_back(attachment.Image->GetDescriptorImageInfo().imageView);
		}

		if (m_DepthAttachment)
		{
			attachmentViews.push_back(m_DepthAttachment.Image->GetDescriptorImageInfo().imageView);
		}

		// Find max number of layers across attachments
		uint32_t maxLayers = 0;
		for (auto attachment : m_ColorAttachments)
		{
			if (attachment.Image->GetSpecification().LayerCount > maxLayers)
			{
				maxLayers = attachment.Image->GetSpecification().LayerCount;
			}
		}

		// Create framebuffer
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.pAttachments = attachmentViews.data();
		framebufferInfo.attachmentCount = (uint32_t)attachmentViews.size();
		framebufferInfo.width = m_Width;
		framebufferInfo.height = m_Height;
		framebufferInfo.layers = maxLayers;

		VK_CHECK_RESULT(vkCreateFramebuffer(device->GetLogicalDevice(), &framebufferInfo, nullptr, &m_Framebuffer));
		VkTools::SetFramebufferName(m_Framebuffer, m_Specification.DebugName.c_str());
	}

}