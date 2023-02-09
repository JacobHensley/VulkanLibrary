#include "pch.h"
#include "Input.h"
#include "Core/Application.h"
#include <GLFW/glfw3.h>

namespace VkLibrary {

	Input* Input::s_Instance = new Input();

	bool Input::IsKeyPressed(int keycode)
	{
		GLFWwindow* window = Application::GetWindow()->GetWindowHandle();
		return glfwGetKey(window, keycode);
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		GLFWwindow* window = Application::GetWindow()->GetWindowHandle();
		return glfwGetMouseButton(window, button);
	}

	glm::vec2 Input::GetMousePosition()
	{
		GLFWwindow* window = Application::GetWindow()->GetWindowHandle();
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		return glm::vec2(x, y);
	}

	bool Input::IsMouseScrolling()
	{
		Ref<Window> window = Application::GetWindow();
		return window->IsMouseScrolling();;
	}

	float Input::GetMouseScrollwheel()
	{
		Ref<Window> window = Application::GetWindow();
		return window->GetMouseScrollwheel();
	}

}