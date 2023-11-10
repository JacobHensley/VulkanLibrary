#pragma once
#include <glm/glm.hpp>

namespace VkLibrary {

	enum class CameraType
	{
		NONE = -1, EDITOR, POV
	};

	struct CameraSpecification
	{
		float verticalFOV = 45.0f;
		float nearClip = 0.1f;
		float farClip = 100.0f;

		CameraType cameraType = CameraType::POV;

		// Editor camera settings
		float panSpeed = 0.001f;
		float rotationSpeed = 0.001f;
		float zoomSpeed = 2.0f;
		float distance = 5.0f;
		float pitch = glm::radians(25.0f);
		float yaw = glm::radians(35.0f);
		glm::vec3 focalPoint = glm::vec3(0.0f);

		// POV camera settings
		float forwardBackwardSpeed = 0.1;
		float leftRightSpeed = 0.1;
		float upDownSpeed = 0.1;
		float lookSpeed = 0.002f;
	};

	class Camera
	{
	public:
		Camera(CameraSpecification specification);

	public:
		bool Update();
		void Reset();
		bool Resize(uint32_t width, uint32_t height);

		const glm::mat4& GetProjection() const { return m_Projection; }
		const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }

		const glm::mat4& GetView() const { return m_View; }
		const glm::mat4& GetInverseView() const { return m_InverseView; }

		const glm::mat4& GetViewProjection() const { return m_ViewProjection; }
		const glm::mat4& GetInverseViewProjection() const { return m_InverseViewProjection; }

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; }

		const glm::vec3& GetRotation() const { return m_Rotation; }

		const glm::vec2 GetPitchYaw() const { return { m_Pitch, m_Yaw }; }
		const glm::vec3& GetFocalPoint() const { return m_FocalPoint; }
		const float GetDistance() const { return m_Distance; }

	private:
		bool HandleEditorCameraMovement(const glm::vec2& delta);
		bool HandlePOVCameraMovement(const glm::vec2& delta);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta, float speed);
		void MouseZoom(float delta);
		
		void RecalculateView();

		glm::vec3 GetUpDirection();
		glm::vec3 GetRightDirection();
		glm::vec3 GetForwardDirection();

		glm::vec3 CalculatePosition();
		glm::quat GetOrientation();

	private:
		glm::mat4 m_Projection{ 1.0f };
		glm::mat4 m_InverseProjection{ 1.0f };

		glm::mat4 m_View{ 1.0f };
		glm::mat4 m_InverseView{ 1.0f };

		glm::mat4 m_ViewProjection{ 1.0f };
		glm::mat4 m_InverseViewProjection{ 1.0f };

		glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f};
		glm::vec3 m_Rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint{ 0.0f, 0.0f, 0.0f };

		float m_Distance = 0.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;

		float m_VerticalFOV = 0.0f;
		float m_NearClip = 0.0f;
		float m_FarClip = 0.0f;

		float m_PanSpeed = 0.0f;
		float m_RotationSpeed = 0.0f;
		float m_ZoomSpeed = 0.0f;

		glm::vec2 m_InitialMousePosition = {0, 0};
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		CameraSpecification m_Specification;
	};

}