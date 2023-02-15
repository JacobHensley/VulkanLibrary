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
		m_Window->SetResizeCallback([this](uint32_t width, uint32_t height) { OnWindowResize(width, height); });
		m_Window->InitVulkanSurface();
		m_VulkanDevice = CreateRef<VulkanDevice>();
		m_Swapchain = CreateRef<Swapchain>();
		VulkanAllocator::Init(m_VulkanDevice);

		m_ImGUIContext = CreateRef<ImGuiLayer>();
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

	void Application::OnWindowResize(uint32_t width, uint32_t height)
	{
		if (!m_Window->IsMinimized())
			m_Swapchain->Resize(width, height);
	}

	void Application::OnImGUIRender()
	{
		m_ImGUIContext->BeginFrame();

		for (auto& layer : m_Layers)
		{
			layer->OnImGUIRender();
		}

		m_ImGUIContext->EndFrame();
	}

	void Application::Run()
	{
		while (!m_Window->IsClosed())
		{
			OnUpdate();
			OnImGUIRender();
			
			if (!m_Window->IsMinimized())
			{
				m_Swapchain->BeginFrame();

				OnRender();

				m_ImGUIContext->RenderDrawLists();
				m_Swapchain->Present();
			}
		}

		vkDeviceWaitIdle(m_VulkanDevice->GetLogicalDevice());
	}

}