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

		// Create image
		ImageSpecification imageSpecification = {};
		imageSpecification.Data = data;
		imageSpecification.Width = width;
		imageSpecification.Height = height;
		imageSpecification.Format = ImageFormat::RGBA8;
		imageSpecification.Usage = ImageUsage::TEXTURE_2D;
		imageSpecification.DebugName = (m_Specification.DebugName + ", Image").c_str();

		m_Image = CreateRef<Image>(imageSpecification);

		// Free CPU memory
		stbi_image_free(data);
	}

	Texture2D::~Texture2D()
	{
	}

}