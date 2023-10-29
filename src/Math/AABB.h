#pragma once
#include "Core/Core.h"
#include <glm/glm.hpp>

namespace VkLibrary {

	struct AABB
	{
		glm::vec3 Min, Max;

		AABB()
			: Min(0), Max(0) 
		{
		}

		AABB(const glm::vec3& min, const glm::vec3& max)
			: Min(min), Max(max)
		{
		//	ASSERT(min.x < max.x && min.y < max.y && min.z < max.z, "Min must be less than max!");
		}

		glm::vec3 GetSize() const
		{
			return Max - Min;
		}
	};

}
