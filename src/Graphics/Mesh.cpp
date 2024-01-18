#include "pch.h"
#include "Mesh.h"

namespace VkLibrary {
	
	Mesh::Mesh(Ref<MeshSource> source)
		: m_MeshSource(source)
	{
		for (const auto& data : m_MeshSource->GetMaterialData())
		{
			MaterialBuffer& buffer = m_MaterialBuffers.emplace_back();
			buffer.data = data;
		}
	}

}