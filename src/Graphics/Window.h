#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

namespace VkLibrary {

	class Window
	{
	public:
		Window(const std::string& name, int width, int height);
		~Window();

	public:
		void OnUpdate();
		void InitVulkanSurface();

		glm::vec2 GetFramebufferSize();

		bool IsMinimized() { return m_Minimized; }
		bool IsClosed();

		void SetMouseCursorMode(bool enable);

		void SetResizeCallback(const std::function<void(uint32_t, uint32_t)>& func) { m_ResizeCallback = func; }

		inline GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
		inline VkSurfaceKHR GetVulkanSurface() const { return m_VulkanSurface; }

	private:
		void Init();

		void OnResize(uint32_t width, uint32_t height);
	
		inline bool IsMouseScrolling() { return m_IsMouseScrolling; }
		inline float GetMouseScrollwheel() { return m_MouseScrollWheel; }
		inline void SetMouseScrollwheel(float value) { m_MouseScrollWheel = value; }

	private:
		const std::string m_Name;
		int m_Width;
		int m_Height;
		bool m_Minimized = false;

		float m_MouseScrollWheel = 0.0f;
		bool m_IsMouseScrolling = false;

		GLFWwindow* m_WindowHandle = nullptr;
		VkSurfaceKHR m_VulkanSurface = nullptr;

		std::function<void(uint32_t, uint32_t)> m_ResizeCallback;

		friend class Input;
	};

}
