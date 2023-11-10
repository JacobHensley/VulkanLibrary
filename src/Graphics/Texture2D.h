#pragma once
#include "Core/Core.h"
#include "Image.h"

namespace VkLibrary {

	struct Texture2DSpecification
	{
		std::filesystem::path path;
		bool sRGB = false;

		std::string DebugName = "Texture2D";
	};

	// TODO: Add support for multiple texture formats

	class Texture2D
	{
	public:
		Texture2D(Texture2DSpecification specification);
		~Texture2D();

		inline Ref<Image> GetImage() { return m_Image; }

		inline const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_Image->GetDescriptorImageInfo(); }
		inline const Texture2DSpecification& GetSpecification() const { return m_Specification; }

	private:
		std::filesystem::path m_Path;
		Ref<Image> m_Image;

		Texture2DSpecification m_Specification;
	};

}