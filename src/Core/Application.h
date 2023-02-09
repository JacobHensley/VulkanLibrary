#pragma once
#include "Core.h"
#include "Layer.h"
#include "Graphics/VulkanInstance.h"
#include "Graphics/VulkanDevice.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Window.h"
#include "ImGui/ImGuiContext.h"

namespace VkLibrary {

	// TODO: Draw ImGUI inside of a render pass at end of OnImGUIRender()

	class Application
	{
	public:
		Application(const std::string name);
		~Application();

	public:
		void Run();

		inline void AddLayer(Ref<Layer> layer) { m_Layers.push_back(layer); }

		inline static Application& GetApp() { return *s_Instance; }

		inline static Ref<Swapchain> GetSwapchain() { return s_Instance->GetSwapchainInternal(); }
		inline static Ref<VulkanInstance> GetVulkanInstance() { return s_Instance->GetVulkanInstanceInternal(); }
		inline static Ref<VulkanDevice> GetVulkanDevice() { return s_Instance->GetVulkanDeviceInternal();; }
		inline static Ref<Window> GetWindow() { return s_Instance->GetWindowInternal();; }

	private:
		void Init();
		void OnUpdate();
		void OnRender();
		void OnImGUIRender();

		inline Ref<Swapchain> GetSwapchainInternal() { return m_Swapchain; }
		inline Ref<VulkanInstance> GetVulkanInstanceInternal() { return m_VulkanInstance; }
		inline Ref<VulkanDevice> GetVulkanDeviceInternal() { return m_VulkanDevice; }
		inline Ref<Window> GetWindowInternal() { return m_Window; }

	private:
		std::string m_Name;
		std::vector<Ref<Layer>> m_Layers;

	private:
		static Application* s_Instance;

		Ref<VulkanInstance> m_VulkanInstance;
		Ref<VulkanDevice> m_VulkanDevice;
		Ref<Swapchain> m_Swapchain;
		Ref<Window> m_Window;
		Ref<ImGuiLayer> m_ImGUIContext;
	};

}