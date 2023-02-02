#include "pch.h"
#include "Application.h"
#include "Graphics/VulkanAllocator.h"

namespace VkLibrary {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string name)
		: m_Name(name)
	{
		Init();
	}

	Application::~Application()
	{
		for (auto& layer : m_Layers)
		{
			layer.reset();
		}

		m_ImGUIContext.reset();
		m_Swapchain.reset();
		VulkanAllocator::Shutdown();
		m_VulkanDevice.reset();
		m_Window.reset();
		m_VulkanInstance.reset();
	}

	void Application::Init()
	{
		ASSERT(!s_Instance, "Instance of Application already exists");
		s_Instance = this;

		Log::Init();

		m_Window = CreateRef<Window>(m_Name, 1280, 720);
		m_VulkanInstance = CreateRef<VulkanInstance>(m_Name);
		m_Window->InitVulkanSurface();
		m_VulkanDevice = CreateRef<VulkanDevice>();
		m_Swapchain = CreateRef<Swapchain>();
		VulkanAllocator::Init(m_VulkanDevice);

		m_ImGUIContext = CreateRef<ImGuiContext>();
	}

	void Application::OnUpdate()
	{
		m_Window->OnUpdate();

		for (auto& layer : m_Layers)
		{
			layer->OnUpdate();
		}
	}

	void Application::OnRender()
	{
		for (auto& layer : m_Layers)
		{
			layer->OnRender();
		}
	}

	void Application::OnImGUIRender()
	{
		m_ImGUIContext->BeginFrame();

		for (auto& layer : m_Layers)
		{
			layer->OnImGUIRender();
		}

		m_ImGUIContext->EndFrame();

		// TODO: Draw ImGUI inside of a render pass
	}

	void Application::Run()
	{
		while (!m_Window->IsClosed())
		{
			OnUpdate();
			
			m_Swapchain->BeginFrame();

			OnRender();
			OnImGUIRender();

			// End frame
			m_Swapchain->Present();
		}

		vkDeviceWaitIdle(m_VulkanDevice->GetLogicalDevice());
	}

}