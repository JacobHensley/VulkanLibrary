#include "pch.h"
#include "Texture2D.h"
#include <stb/stb_image.h>

namespace VkLibrary {

	Texture2D::Texture2D(Texture2DSpecification specification)
		: m_Specification(specification)
	{
		m_Path = m_Specification.path;

		// Load image from disk
		int width, height, bpp;
		stbi_set_flip_vertically_on_load(false);

		uint8_t* data = stbi_load(m_Specification.path.string().c_str(), &width, &height, &bpp, 4);
		ASSERT(data, "Failed to load image");

		// Set width and height
		m_Width = width;
		m_Height = height;

		// Create image
		ImageSpecification imageSpecification = {};
		imageSpecification.Data = data;
		imageSpecification.Width = m_Width;
		imageSpecification.Height = m_Width;
		imageSpecification.Format = VK_FORMAT_R8G8B8A8_UNORM;
		imageSpecification.DebugName = (m_Specification.DebugName + ", Image").c_str();

		m_Image = CreateRef<Image>(imageSpecification);

		// Free CPU memory
		stbi_image_free(data);
	}

	Texture2D::~Texture2D()
	{
	}

}