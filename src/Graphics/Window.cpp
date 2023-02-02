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
    }

    void Window::OnUpdate()
    {
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