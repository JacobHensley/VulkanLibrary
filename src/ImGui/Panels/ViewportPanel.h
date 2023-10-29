#pragma once
#include "Core/Core.h"
#include "Graphics/Camera.h"
#include "Graphics/Image.h"
#include "Math/Ray.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace VkLibrary {

	class ViewportPanel
	{
	public:
		ViewportPanel();

	public:
		void Render(Ref<Image> image);

		glm::vec2 GetMouseNDC();
		Ray CastMouseRay(Ref<Camera> camera);

		const glm::vec2& GetSize() const { return m_Size; }
		const glm::vec2& GetPosition() const { return m_Position; }

		bool HasResized() const { return m_Resized; }
		bool IsHovered() const { return m_Hovered; }
		bool IsFocused() const { return m_Focused; }

	private:
		glm::vec2 m_Size = { 0.0f, 0.0f };
		glm::vec2 m_Position = { 0.0f, 0.0f };

		bool m_Resized = false;
		bool m_Hovered = false;
		bool m_Focused = false;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_ImageDescriptorSet = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
	};

}