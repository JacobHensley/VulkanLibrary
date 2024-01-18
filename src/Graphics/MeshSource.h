#pragma once
#include "Math/Triangle.h"
#include "Math/AABB.h"
#include "Math/Ray.h"
#include "VulkanBuffers.h"
#include "Texture.h"
#include "Material.h"
#include <tinygltf/tiny_gltf.h>
#include <glm/glm.hpp>
#include <filesystem>

namespace VkLibrary {

	struct Vertex
	{
		glm::vec3 Position{ 0.0f };
		glm::vec2 TextureCoords{ 0.0f };
		glm::vec3 Normal{ 0.0f };
		glm::vec4 Tangent{ 0.0f };
	};

	struct SubMesh
	{
		uint32_t VertexOffset = 0;
		uint32_t VertexCount = 0;
		uint32_t IndexOffset = 0;
		uint32_t IndexCount = 0;
		uint32_t TriangleOffset = 0;
		uint32_t TriangleCount = 0;
		uint32_t MaterialIndex = 0;
		AABB BoundingBox = AABB();
		glm::mat4 LocalTransform = glm::mat4(1.0f);
		glm::mat4 WorldTransform = glm::mat4(1.0f);
	};

	struct MaterialData
	{
		glm::vec3 AlbedoValue{ 0.8f };
		float MetallicValue = 0.0f;
		float RoughnessValue = 1.0f;
		glm::vec3 EmissiveValue{ 0.0f };
		float EmissiveStrength = 1.0f;

		uint32_t UseNormalMap = 0;

		int AlbedoMapIndex = -1;
		int MetallicRoughnessMapIndex = -1;
		int NormalMapIndex = -1;
	};

	// TODO: Pull m_DefaultShader from some shader libary instead of creating one for every mesh

	class MeshSource
	{
	public:
		MeshSource(const std::string_view path);
		~MeshSource() = default;
	public:
		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
		
		inline const std::vector<Triangle>& GetTriangles() const { return m_Triangles; };
		inline std::vector<MaterialData>& GetMaterialData() { return m_MaterialBuffers; };
		inline const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }

		inline Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

		int RayIntersection(Ray ray, const glm::mat4& transform);

	private:
		void Init();

		void LoadVertexData();
		void LoadMaterialData();
		void CalculateNodeTransforms(const tinygltf::Node& inputNode, const tinygltf::Model& input, const glm::mat4& parentTransform);

	private:
		std::filesystem::path m_Path;

		std::vector<SubMesh> m_SubMeshes;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		
		std::vector<Triangle> m_Triangles;
		AABB m_BoundingBox;

		std::vector<MaterialData> m_MaterialBuffers;
		std::vector<Ref<Texture2D>> m_Textures;

		Ref<Shader> m_DefaultShader;

		tinygltf::Model m_Model;
	};

}