#include "pch.h"
#include "Window.h"
#include "Core/Application.h"

namespace VkLibrary {

    Window::Window(const std::string& name, int width, int height)
	    : m_Name(name), m_Width(width), m_Height(height)
    {
        Init();
    }

    Window::~Window()
    {
        VkInstance instance = Application::GetVulkanInstance()->GetInstanceHandle();
        vkDestroySurfaceKHR(instance, m_VulkanSurface, nullptr);

        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate();
    }

    void Window::Init()
    {
        bool initialized = glfwInit();
        ASSERT(initialized, "Failed to initialize glfw window");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_WindowHandle = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_WindowHandle, this);

        glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            Window& win = *(Window*)glfwGetWindowUserPointer(window);
            win.SetMouseScrollwheel(yOffset);
            win.m_IsMouseScrolling = true;
        });

        glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
        {
			Window& win = *(Window*)glfwGetWindowUserPointer(window);
            win.OnResize((uint32_t)width, (uint32_t)height);
        });
    }

	void Window::OnResize(uint32_t width, uint32_t height)
	{
        m_Width = (int)width;
        m_Height = (int)height;
        m_Minimized = m_Width == 0 || m_Height == 0;

        if (m_ResizeCallback)
            m_ResizeCallback(width, height);
	}

	void Window::OnUpdate()
    {
        m_IsMouseScrolling = false;
        glfwPollEvents();
    }

    void Window::InitVulkanSurface()
    {
        VkInstance instance = Application::GetApp().GetVulkanInstance()->GetInstanceHandle();

        glfwCreateWindowSurface(instance, m_WindowHandle, nullptr, &m_VulkanSurface);
    }

    glm::vec2 Window::GetFramebufferSize()
    {
        glm::ivec2 result;
        glfwGetFramebufferSize(m_WindowHandle, &result.x, &result.y);
        return result;
    }

    bool Window::IsClosed()
    {
        return glfwWindowShouldClose(m_WindowHandle);
    }

}