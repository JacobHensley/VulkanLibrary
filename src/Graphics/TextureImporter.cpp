#include "pch.h"
#include "TextureImporter.h"
#include "Memory/FileIO.h"

namespace VkLibrary
{
	TextureImporter::TextureImporter(const std::filesystem::path& path)
		: m_Path(path)
	{
	}

	Ref<Texture2D> TextureImporter::ImportTexture2D()
	{	
		FileReader reader(m_Path);
		TextureFileHeader header = reader.ReadRaw<TextureFileHeader>();
		Buffer buffer = reader.ReadBuffer();

		ImageSpecification spec;
		spec.Width = header.Width;
		spec.Height = header.Height;
		spec.Format = (ImageFormat)header.Format;

		Ref<Image> image = CreateRef<Image>(spec, buffer);
		Ref<Texture2D> texture = CreateRef<Texture2D>(image);

		return texture;
	}

	void TextureImporter::SerializeTexture2D(Ref<Texture2D> texture)
	{
		FileWriter writer(m_Path);
		TextureFileHeader header;
		header.Width = texture->GetImage()->GetWidth();
		header.Height = texture->GetImage()->GetHeight();
		header.Format = (uint32_t)texture->GetImage()->GetSpecification().Format;
		writer.WriteRaw(header);

		writer.WriteBuffer(texture->GetImage()->GetBuffer());
	}
}