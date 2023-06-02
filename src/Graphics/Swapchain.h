#pragma once
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct SwapchainImage
	{
		VkImage Image;
		VkImageView ImageView;
	};

	class Swapchain
	{
	public:
		Swapchain();
		~Swapchain();

	public:
		void BeginFrame();
		void Present();

		inline VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
		inline VkRenderPass GetRenderPass() const { return m_RenderPass; }

		inline const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }
		inline VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_CurrentImageIndex]; }
		inline uint32_t GetCurrentBufferIndex() const { return m_CurrentBufferIndex; }

		inline const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_CommandBuffers; }
		inline VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentBufferIndex]; }
		inline uint32_t GetCurrentFrameIndex() { return m_CurrentImageIndex; }

		inline VkCommandPool GetCommandPool() const { return m_CommandPool; }
		inline uint32_t GetImageCount() const { return m_DesiredImageCount; }
		inline VkExtent2D GetExtent() const { return m_Extent; }

		uint32_t GetFramesInFlight();
		void Resize(uint32_t width, uint32_t height);

	private:
		void Init(uint32_t width, uint32_t height);
		void Destroy();

		void PickDetails(uint32_t width, uint32_t height);
		void CreateImageViews();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSynchronizationObjects();

		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		uint32_t m_Width = 0, m_Height = 0;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<SwapchainImage> m_Images;
		std::vector<VkFramebuffer> m_Framebuffers;

		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentBufferIndex = 0;
		uint32_t m_ImageSemaphoreIndex = 0;

		std::vector<VkSemaphore> m_PresentCompleteSemaphores;
		std::vector<VkSemaphore> m_RenderCompleteSemaphores;
		std::vector<VkFence> m_WaitFences;

		VkSurfaceFormatKHR m_ImageFormat;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_Extent;
		uint32_t m_DesiredImageCount;

		uint32_t m_ImageCount;
	};

}