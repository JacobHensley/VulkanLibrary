#pragma once
#include "MeshSource.h"

namespace VkLibrary {

	struct MaterialBuffer
	{
		MaterialData data;

		float Anisotropic = 0.0f;
		float Subsurface = 0.0f;
		float SpecularTint = 0.0f;
		float Sheen = 0.0f;
		float SheenTint = 0.0f;
		float Clearcoat = 0.0f;
		float ClearcoatRoughness = 0.0f;
		float SpecTrans = 0.0f;
		float ior = 1.5f;
	};

	class Mesh
	{
	public:
		Mesh(Ref<MeshSource> source);
		~Mesh() = default;

		inline std::vector<MaterialBuffer>& GetMaterialBuffers() { return m_MaterialBuffers; };
		Ref<MeshSource> GetSource() const { return m_MeshSource; }

	public:
		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_MeshSource->GetSubMeshes(); }
		inline std::vector<SubMesh>& GetSubMeshes() { return m_MeshSource->GetSubMeshes(); }

		inline const std::vector<Triangle>& GetTriangles() const { return m_MeshSource->GetTriangles(); };
		inline std::vector<MaterialData>& GetMaterialData() { return m_MeshSource->GetMaterialData(); };
		inline const std::vector<Ref<Texture2D>>& GetTextures() const { return m_MeshSource->GetTextures(); }

		inline Ref<VertexBuffer> GetVertexBuffer() const { return m_MeshSource->GetVertexBuffer(); }
		inline Ref<IndexBuffer> GetIndexBuffer() const { return m_MeshSource->GetIndexBuffer(); }

		int RayIntersection(Ray ray, const glm::mat4& transform) { return m_MeshSource->RayIntersection(ray, transform); };

	private:
		Ref<MeshSource> m_MeshSource;
		std::vector<MaterialBuffer> m_MaterialBuffers;
	};

}