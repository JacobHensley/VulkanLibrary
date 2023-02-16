#include "pch.h"
#include "Mesh.h"
#include "Core/Core.h"
#include <glm/gtc/type_ptr.hpp>

namespace VkLibrary {
	
	Mesh::Mesh(const std::string_view path)
		: m_Path(path)
	{
		Init();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Init()
	{
		tinygltf::TinyGLTF loader;
		std::string error;
		std::string warning;

		if (m_Path.extension() == ".gltf")
		{
			loader.LoadASCIIFromFile(&m_Model, &error, &warning, m_Path.string());
		}	
		else if (m_Path.extension() == ".glb")
		{
			loader.LoadBinaryFromFile(&m_Model, &error, &warning, m_Path.string());
		}
		else
		{
			ASSERT(false, "Model file is not in correct format");
		}
			
		ASSERT(warning.empty(), warning);
		ASSERT(error.empty(), error);

		LoadVertexData();
		LoadMaterialData();

		const tinygltf::Scene& scene = m_Model.scenes[0];
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			const tinygltf::Node node = m_Model.nodes[scene.nodes[i]];
			CalculateNodeTransforms(node, m_Model, glm::mat4(1.0f));
		}

		m_VertexBuffer = CreateRef<VertexBuffer>(m_Vertices.data(), sizeof(Vertex) * m_Vertices.size());
		m_IndexBuffer = CreateRef<IndexBuffer>(m_Indices.data(), sizeof(uint32_t) * m_Indices.size(), m_Indices.size());
	}

	void Mesh::LoadVertexData()
	{
		m_SubMeshes.reserve(m_Model.meshes.size());

		int subMeshVertexOffset = 0;
		int subMeshIndexOffset = 0;

		for (int i = 0; i < m_Model.meshes.size(); i++)
		{
			tinygltf::Mesh mesh = m_Model.meshes[i];

			for (int i = 0; i < mesh.primitives.size(); i++)
			{
				int subMeshVertexCount = 0;
				int subMeshIndexCount = 0;

				const tinygltf::Primitive& primitive = mesh.primitives[i];

				// Positions
				if (primitive.attributes.find("POSITION") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];

					subMeshVertexCount = accessor.count;
					if (m_Vertices.size() < subMeshVertexOffset + subMeshVertexCount)
					{
						m_Vertices.resize(subMeshVertexOffset + subMeshVertexCount);
					}

					const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].Position.x = positions[j * 3 + 0];
						m_Vertices[subMeshVertexOffset + j].Position.y = positions[j * 3 + 1];
						m_Vertices[subMeshVertexOffset + j].Position.z = positions[j * 3 + 2];
					}
				}

				// Normals
				if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];
					
					const float* normals = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].Normal.x = normals[j * 3 + 0];
						m_Vertices[subMeshVertexOffset + j].Normal.y = normals[j * 3 + 1];
						m_Vertices[subMeshVertexOffset + j].Normal.z = normals[j * 3 + 2];
					}
				}

				// Tangents
				if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[primitive.attributes.find("TANGENT")->second];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];

					const float* tangents = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].Tangent.x = tangents[j * 4 + 0];
						m_Vertices[subMeshVertexOffset + j].Tangent.y = tangents[j * 4 + 1];
						m_Vertices[subMeshVertexOffset + j].Tangent.z = tangents[j * 4 + 2];
						m_Vertices[subMeshVertexOffset + j].Tangent.w = tangents[j * 4 + 3];
					}
				}

				// Texture Coordinates
				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];

					const float* texCoords = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].TextureCoords.x = texCoords[j * 2 + 0];
						m_Vertices[subMeshVertexOffset + j].TextureCoords.y = texCoords[j * 2 + 1];
					}
				}

				// Indices
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[primitive.indices];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];

					subMeshIndexCount = accessor.count;
					if (m_Indices.size() < subMeshIndexOffset + subMeshIndexCount)
					{
						m_Indices.resize(subMeshIndexOffset + subMeshIndexCount);
					}

					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					{
						const uint32_t* indices = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						for (int j = 0; j < accessor.count; j++)
						{
							m_Indices[subMeshIndexOffset + j] = indices[j];
						}	
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						const uint16_t* indices = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						for (int j = 0; j < accessor.count; j++)
						{
							m_Indices[subMeshIndexOffset + j] = (uint32_t)indices[j];
						}	
					}
					else
					{
						ASSERT(false, "Mesh indices are not in the correct format");
					}
				}

				SubMesh& subMesh = m_SubMeshes.emplace_back();
				subMesh.IndexCount = subMeshIndexCount;
				subMesh.IndexOffset = subMeshIndexOffset;
				subMesh.VertexCount = subMeshVertexCount;
				subMesh.VertexOffset = subMeshVertexOffset;
				subMesh.MaterialIndex = primitive.material;

				subMeshIndexOffset += subMeshIndexCount;
				subMeshVertexOffset += subMeshVertexCount;
			}
		}
	}

	void Mesh::LoadMaterialData()
	{
		for (tinygltf::Material& material : m_Model.materials)
		{
		}
	}

	void Mesh::CalculateNodeTransforms(const tinygltf::Node& node, const tinygltf::Model& scene, const glm::mat4& parentTransform)
	{
		glm::mat4 transform = glm::mat4(1.0f);

		if (node.translation.size() == 3) 
		{
			transform = glm::translate(transform, glm::vec3(glm::make_vec3(node.translation.data())));
		}
		if (node.rotation.size() == 4) 
		{
			glm::quat q = glm::make_quat(node.rotation.data());
			transform *= glm::mat4(q);
		}
		if (node.scale.size() == 3) 
		{
			transform = glm::scale(transform, glm::vec3(glm::make_vec3(node.scale.data())));
		}
		if (node.matrix.size() == 16) 
		{
			transform = glm::make_mat4x4(node.matrix.data());
		};

		m_SubMeshes[node.mesh].WorldTransform = parentTransform * transform;
		m_SubMeshes[node.mesh].LocalTransform = transform;

		if (node.children.size() > 0) 
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				CalculateNodeTransforms(scene.nodes[node.children[i]], scene, m_SubMeshes[node.mesh].WorldTransform);
			}
		}
	}

}