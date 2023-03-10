#pragma once
#include "Graphics/VulkanBuffers.h"
#include "Graphics/Texture2D.h"
#include <tinygltf/tiny_gltf.h>
#include <glm/glm.hpp>
#include <filesystem>

namespace VkLibrary {

	struct Vertex
	{
		glm::vec3 Position{ 0.0f };
		glm::vec3 Normal{ 0.0f };
		glm::vec2 TextureCoords{ 0.0f };
		glm::vec4 Tangent{ 0.0f };
	};

	struct SubMesh
	{
		uint32_t VertexOffset = 0;
		uint32_t VertexCount = 0;
		uint32_t IndexOffset = 0;
		uint32_t IndexCount = 0;
		uint32_t MaterialIndex = 0;
		glm::mat4 LocalTransform = glm::mat4(1.0f);
		glm::mat4 WorldTransform = glm::mat4(1.0f);
	};

	struct MaterialBuffer
	{
		glm::vec3 AlbedoValue{ 0.8f };
		float MetallicValue = 0.0f;
		float RoughnessValue = 1.0f;
		uint32_t UseNormalMap = 0;

		uint32_t AlbedoMapIndex = 0;
		uint32_t NormalMapIndex = 0;
		uint32_t MetallicRoughnessMapIndex = 0;
	};

	class Mesh
	{
	public:
		Mesh(const std::string_view path);
		~Mesh();

	public:
		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
		
		inline const std::vector<MaterialBuffer>& GetMaterialBuffers() const { return m_MaterialBuffers; };
		inline const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }

		inline Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

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

		std::vector<MaterialBuffer> m_MaterialBuffers;
		std::vector<Ref<Texture2D>> m_Textures;

		tinygltf::Model m_Model;
	};

}