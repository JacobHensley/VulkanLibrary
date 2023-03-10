#include "pch.h"
#include "Camera.h"
#include "Input/Input.h"
#include "Input/KeyCodes.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define PI 3.14159f

namespace VkLibrary {

	Camera::Camera(CameraSpecification specification)
		: m_Specification(specification)
	{
		Reset();
	}

	void Camera::Update()
	{
		glm::vec2 mousePosition = Input::GetMousePosition();
		glm::vec2 delta = mousePosition - m_InitialMousePosition;
		m_InitialMousePosition = mousePosition;

		bool moved = false;

		if (Input::IsKeyPressed(KEY_LEFT_SHIFT) && Input::IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
		{
			MousePan(delta);
			moved = true;
		}
		else if (Input::IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
		{
			MouseRotate(delta);
			moved = true;
		}

		if (Input::IsMouseScrolling())
		{
			MouseZoom(Input::GetMouseScrollwheel());
			moved = true;
		}

		m_Position = CalculatePosition();
		m_Rotation = glm::eulerAngles(GetOrientation()) * (180.0f / (float)PI);

		if (moved)
		{
			RecalculateView();
		}
	}

	void Camera::Reset()
	{
		m_VerticalFOV = m_Specification.verticalFOV;
		m_NearClip = m_Specification.nearClip;
		m_FarClip = m_Specification.farClip;

		m_PanSpeed = m_Specification.panSpeed;
		m_RotationSpeed = m_Specification.rotationSpeed;
		m_ZoomSpeed = m_Specification.zoomSpeed;

		m_FocalPoint = m_Specification.focalPoint;
		m_Distance = m_Specification.distance;

		m_Yaw = m_Specification.yaw * (PI / 180.0f);
		m_Pitch = m_Specification.pitch * (PI / 180.0f);

		m_Position = CalculatePosition();
		m_Rotation = glm::eulerAngles(GetOrientation()) * (180.0f / (float)PI);
	}

	void Camera::Resize(uint32_t width, uint32_t height)
	{
		if (width == m_ViewportWidth && height == m_ViewportHeight)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
		m_InverseProjection = glm::inverse(m_Projection);

		RecalculateView();
	}

	void Camera::MousePan(const glm::vec2& delta)
	{
		m_FocalPoint += -GetRightDirection() * delta.x * m_PanSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * m_PanSpeed * m_Distance;
	}

	void Camera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * m_RotationSpeed;
		m_Pitch += delta.y * m_RotationSpeed;
	}

	void Camera::MouseZoom(float delta)
	{
		m_Distance -= delta * m_ZoomSpeed;
	}

	void Camera::RecalculateView()
	{
		glm::quat orientation = GetOrientation();

		m_View = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1)) * glm::toMat4(glm::conjugate(orientation)) * glm::translate(glm::mat4(1.0f), -m_Position);
		m_InverseView = glm::inverse(m_View);

		m_ViewProjection = m_Projection * m_View;
		m_InverseViewProjection = glm::inverse(m_ViewProjection);
	}

	glm::vec3 Camera::GetUpDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::GetRightDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 Camera::GetForwardDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::CalculatePosition()
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat Camera::GetOrientation()
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

}