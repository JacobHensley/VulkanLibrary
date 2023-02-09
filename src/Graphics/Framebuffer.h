#pragma once
#include "Image.h"

namespace VkLibrary {

	struct FramebufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		float Scale = 1.0f;
		bool ClearOnLoad = true;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		std::vector<VkFormat> AttachmentFormats;
		
		std::string DebugName = "Framebuffer";
	};

	struct FramebufferAttachment
	{
		Ref<Image> Image;
		VkAttachmentDescription Description;

		operator bool() const { return (bool)Image; }
	};

	// TODO: Check a resource release queue or intrusive refrence counting system before releasing framebuffer
	// TODO: Don't use a staging buffer for attachment images
	
	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& specification);
		~Framebuffer();

	public:
		void Resize(uint32_t width, uint32_t height);
		void CreateFramebuffer();

		inline VkFramebuffer GetFramebuffer() { return m_Framebuffer; }
		inline VkRenderPass GetRenderPass() { return m_RenderPass; }

		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }

		Ref<Image> GetImage(uint32_t attachmentIndex) { return m_ColorAttachments[attachmentIndex].Image; }
		Ref<Image> GetDepthImage() const { return m_DepthAttachment.Image; }

		uint32_t GetColorAttachmentCount() const { return m_ColorAttachmentCount; }
		bool HasDepthAttachment() const { return m_DepthAttachment; }

		inline const FramebufferSpecification& GetSpecification() const { return m_Specification; }

	private:
		void InitAttachmentImages();

	private:
		FramebufferSpecification m_Specification;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		uint32_t m_ColorAttachmentCount = 0;
		std::vector<FramebufferAttachment> m_ColorAttachments;
		FramebufferAttachment m_DepthAttachment;
	};

}