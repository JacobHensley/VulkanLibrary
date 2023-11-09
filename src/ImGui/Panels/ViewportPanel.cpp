#include "pch.h"
#include "ViewportPanel.h"
#include "Core/Application.h"
#include "Graphics/VulkanTools.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace VkLibrary {

	ViewportPanel::ViewportPanel()
	{
		m_DescriptorPool = VkTools::CreateDescriptorPool();
		m_ImageDescriptorSet = VkTools::AllocateDescriptorSet(m_DescriptorPool, &ImGui_ImplVulkan_GetDescriptorSetLayout());
	}

	void ViewportPanel::Render(Ref<Image> image)
	{
		m_Resized = false;
		 
		ImGuiIO io = ImGui::GetIO();
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
		bool open = true;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("RTX On", &open, flags);

		ImVec2 size = ImGui::GetContentRegionAvail();
		if (m_Size.x != size.x || m_Size.y != size.y)
		{
			m_Resized = true;
			m_Size = { size.x, size.y };
		}

		ImVec2 position = ImGui::GetWindowPos();
		m_Position = { position.x, position.y };

		ImVec2 windowOffset = ImGui::GetCursorPos();
		m_Position.x += windowOffset.x;
		m_Position.y += windowOffset.y;
		
		m_Hovered = ImGui::IsWindowHovered();
		m_Focused =  ImGui::IsWindowFocused();

		// Update viewport descriptor on reisze
		{
			Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

			const VkDescriptorImageInfo& descriptorInfo = image->GetDescriptorImageInfo();
			if (m_ImageView != descriptorInfo.imageView)
			{
				VkWriteDescriptorSet writeDescriptor = VkTools::WriteDescriptorSet(m_ImageDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &descriptorInfo);
				vkUpdateDescriptorSets(device->GetLogicalDevice(), 1, &writeDescriptor, 0, nullptr);
				m_ImageView = descriptorInfo.imageView;
			}
		}

		ImGui::Image(m_ImageDescriptorSet, ImVec2(size.x, size.y), ImVec2::ImVec2(0, 1), ImVec2::ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();
	}

	glm::vec2 ViewportPanel::GetMouseNDC()
	{
		auto [mouseX, mouseY] = ImGui::GetMousePos();
		mouseX -= m_Position.x;
		mouseY -= m_Position.y;

		glm::vec2 NDC = { (mouseX / m_Size.x) * 2.0f - 1.0f, ((mouseY / m_Size.y) * 2.0f - 1.0f) * -1.0f };

		return NDC;
	}

	Ray ViewportPanel::CastMouseRay(Ref<Camera> camera)
	{
		glm::vec2 NDC = GetMouseNDC();

		glm::vec4 mouseClipPos = { NDC.x, NDC.y, -1.0f, 1.0f };

		glm::mat4 inverseProjection = glm::inverse(camera->GetProjection());
		glm::mat3 inverseView = glm::inverse(glm::mat3(camera->GetView()));

		glm::vec3 cameraPosition = glm::inverse(camera->GetView())[3];

		glm::vec4 ray = inverseProjection * mouseClipPos;
		glm::vec3 rayDirection = inverseView * glm::vec3(ray);

		return Ray(cameraPosition, rayDirection);
	}

}