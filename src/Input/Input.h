#pragma once
#include <glm/glm.hpp>

namespace VkLibrary {

	class Input
	{
	public:
		static bool IsKeyPressed(int keycode);

		static bool IsMouseButtonPressed(int button);
		static glm::vec2 GetMousePosition();

		static bool IsMouseScrolling();
		static float GetMouseScrollwheel();

	private:
		static Input* s_Instance;
	};
}