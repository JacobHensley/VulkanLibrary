#pragma once
#include "Core/Core.h"
#include "Graphics/Texture.h"
#include <filesystem>

namespace VkLibrary
{
	struct TextureFileHeader
	{
		const char HEADER[4] = {'V', 'L', 'T', 'X'};
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t Format = 0;
	};

	class TextureImporter
	{
	public:
		TextureImporter(const std::filesystem::path& path);
		~TextureImporter() = default;
		
		Ref<Texture2D> ImportTexture2D();
		void SerializeTexture2D(Ref<Texture2D> texture);

	private:
		std::filesystem::path m_Path;
	};
}