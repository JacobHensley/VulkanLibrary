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

		m_DefaultShader = CreateRef<Shader>("assets/shaders/PBR.glsl");

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
		int subMeshTriangleOffset = 0;

		m_BoundingBox = AABB({ UINT64_MAX,UINT64_MAX,UINT64_MAX }, { -(float)UINT64_MAX,-(float)UINT64_MAX,-(float)UINT64_MAX });

		for (int i = 0; i < m_Model.meshes.size(); i++)
		{
			tinygltf::Mesh mesh = m_Model.meshes[i];

			for (int i = 0; i < mesh.primitives.size(); i++)
			{
				int subMeshVertexCount = 0;
				int subMeshIndexCount = 0;
				int subMeshTriangleCount = 0;
				AABB subMeshBoundingBox = AABB({ UINT64_MAX,UINT64_MAX,UINT64_MAX }, { -(float)UINT64_MAX,-(float)UINT64_MAX,-(float)UINT64_MAX });

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

						subMeshBoundingBox.Min.x = glm::min(m_Vertices[subMeshVertexOffset + j].Position.x, subMeshBoundingBox.Min.x);
						subMeshBoundingBox.Min.y = glm::min(m_Vertices[subMeshVertexOffset + j].Position.y, subMeshBoundingBox.Min.y);
						subMeshBoundingBox.Min.z = glm::min(m_Vertices[subMeshVertexOffset + j].Position.z, subMeshBoundingBox.Min.z);

						subMeshBoundingBox.Max.x = glm::max(m_Vertices[subMeshVertexOffset + j].Position.x, subMeshBoundingBox.Max.x);
						subMeshBoundingBox.Max.y = glm::max(m_Vertices[subMeshVertexOffset + j].Position.y, subMeshBoundingBox.Max.y);
						subMeshBoundingBox.Max.z = glm::max(m_Vertices[subMeshVertexOffset + j].Position.z, subMeshBoundingBox.Max.z);
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

						for (size_t j = 0; j < accessor.count; j += 3) 
						{
							Triangle& triangle = m_Triangles.emplace_back();
							triangle.Points[0] = m_Vertices[subMeshVertexOffset + indices[j + 0]].Position;
							triangle.Points[1] = m_Vertices[subMeshVertexOffset + indices[j + 1]].Position;
							triangle.Points[2] = m_Vertices[subMeshVertexOffset + indices[j + 2]].Position;
							subMeshTriangleCount++;
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

					for (size_t j = 0; j < accessor.count; j += 3)
					{
						Triangle& triangle = m_Triangles.emplace_back();
						triangle.Points[0] = m_Vertices[subMeshVertexOffset + m_Indices[subMeshIndexOffset + j + 0]].Position;
						triangle.Points[1] = m_Vertices[subMeshVertexOffset + m_Indices[subMeshIndexOffset + j + 1]].Position;
						triangle.Points[2] = m_Vertices[subMeshVertexOffset + m_Indices[subMeshIndexOffset + j + 2]].Position;
						subMeshTriangleCount++;
					}
				}

				m_BoundingBox.Min.x = glm::min(subMeshBoundingBox.Min.x, m_BoundingBox.Min.x);
				m_BoundingBox.Min.y = glm::min(subMeshBoundingBox.Min.y, m_BoundingBox.Min.y);
				m_BoundingBox.Min.z = glm::min(subMeshBoundingBox.Min.z, m_BoundingBox.Min.z);

				m_BoundingBox.Max.x = glm::max(subMeshBoundingBox.Max.x, m_BoundingBox.Max.x);
				m_BoundingBox.Max.y = glm::max(subMeshBoundingBox.Max.y, m_BoundingBox.Max.y);
				m_BoundingBox.Max.z = glm::max(subMeshBoundingBox.Max.z, m_BoundingBox.Max.z);

				SubMesh& subMesh = m_SubMeshes.emplace_back();
				subMesh.IndexCount = subMeshIndexCount;
				subMesh.IndexOffset = subMeshIndexOffset;
				subMesh.VertexCount = subMeshVertexCount;
				subMesh.VertexOffset = subMeshVertexOffset;
				subMesh.TriangleCount = subMeshTriangleCount;
				subMesh.TriangleOffset = subMeshTriangleOffset;
				subMesh.MaterialIndex = primitive.material;
				subMesh.BoundingBox = subMeshBoundingBox;

				subMeshIndexOffset += subMeshIndexCount;
				subMeshVertexOffset += subMeshVertexCount;
				subMeshTriangleOffset += subMeshTriangleCount;
			}
		}
	}

	void Mesh::LoadMaterialData()
	{
		for (tinygltf::Material& gltfMaterial : m_Model.materials)
		{
			MaterialBuffer& material = m_MaterialBuffers.emplace_back();

			// PBR values
			glm::vec3 albedoValue = glm::make_vec3(&gltfMaterial.pbrMetallicRoughness.baseColorFactor[0]);
			float metallicValue = gltfMaterial.pbrMetallicRoughness.metallicFactor;
			float roughnessValue = gltfMaterial.pbrMetallicRoughness.roughnessFactor;
			glm::vec3 emissiveValue = glm::make_vec3(&gltfMaterial.emissiveFactor[0]);

			// Default PBR values
			float emissiveStrength = 1.0f;
			float useNormalMap = 0.0f;

			// TODO: Pick default values
			float ior = 1.0;
			float specularColorFactor = 1.0;
			float clearcoatFactor = 0.0;
			float clearcoatRoughnessFactor = 0.0;

			// Albedo texture
			uint32_t albedoTextureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
			if (albedoTextureIndex != -1)
			{
				const auto& gltfTexture = m_Model.textures[albedoTextureIndex];
				const auto& image = m_Model.images[gltfTexture.source];

				Texture2DSpecification textureSpec;
				textureSpec.path = m_Path.parent_path() / image.uri;
				textureSpec.DebugName = (m_Path.filename().string() + ", Albedo Texture");

				m_Textures.emplace_back(CreateRef<Texture2D>(textureSpec));

				material.AlbedoMapIndex = (uint32_t)m_Textures.size() - 1;
				albedoValue = glm::vec3(1.0f);
			}

			// MetallicRoughness texture
			uint32_t metallicRoughnessTextureIndex = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
			if (metallicRoughnessTextureIndex != -1)
			{
				const auto& gltfTexture = m_Model.textures[metallicRoughnessTextureIndex];
				const auto& image = m_Model.images[gltfTexture.source];

				Texture2DSpecification textureSpec;
				textureSpec.path = m_Path.parent_path() / image.uri;
				textureSpec.DebugName = (m_Path.filename().string() + ", MetallicRoughness Texture");

				m_Textures.emplace_back(CreateRef<Texture2D>(textureSpec));

				material.MetallicRoughnessMapIndex = (uint32_t)m_Textures.size() - 1;
				metallicValue = 1.0f;
				roughnessValue = 1.0f;
			}

			// Normal texture
			uint32_t normalTextureIndex = gltfMaterial.normalTexture.index;
			if (normalTextureIndex != -1)
			{
				const auto& gltfTexture = m_Model.textures[normalTextureIndex];
				const auto& image = m_Model.images[gltfTexture.source];

				Texture2DSpecification textureSpec;
				textureSpec.path = m_Path.parent_path() / image.uri;
				textureSpec.DebugName = (m_Path.filename().string() + ", Normal Texture");

				m_Textures.emplace_back(CreateRef<Texture2D>(textureSpec));

				material.NormalMapIndex = (uint32_t)m_Textures.size() - 1;
				useNormalMap = 1.0f;
			}

			// Load extension data
			if (gltfMaterial.extensions.find("KHR_materials_emissive_strength") != gltfMaterial.extensions.end())
			{
				emissiveStrength = (float)gltfMaterial.extensions.at("KHR_materials_emissive_strength").Get("emissiveStrength").GetNumberAsDouble();
			}

			if (gltfMaterial.extensions.find("KHR_materials_ior") != gltfMaterial.extensions.end())
			{
				ior = (float)gltfMaterial.extensions.at("KHR_materials_ior").Get("ior").GetNumberAsDouble();
			}

			if (gltfMaterial.extensions.find("KHR_materials_specular") != gltfMaterial.extensions.end())
			{
				specularColorFactor = (float)gltfMaterial.extensions.at("KHR_materials_specular").Get("specularColorFactor").GetNumberAsDouble();
			}

			if (gltfMaterial.extensions.find("KHR_materials_clearcoat") != gltfMaterial.extensions.end())
			{
				clearcoatFactor = (float)gltfMaterial.extensions.at("KHR_materials_clearcoat").Get("clearcoatFactor").GetNumberAsDouble();
				clearcoatRoughnessFactor = (float)gltfMaterial.extensions.at("KHR_materials_clearcoat").Get("clearcoatRoughnessFactor").GetNumberAsDouble();
			}

			material.AlbedoValue = albedoValue;
			material.MetallicValue = metallicValue;
			material.RoughnessValue = roughnessValue;
			material.EmissiveValue = emissiveValue;
			material.EmissiveStrength = emissiveStrength;
			material.UseNormalMap =  useNormalMap;
		}
	}

	// NOTE: Very inefficient for this task since we need to find the closest tirangle.
	int Mesh::RayIntersection(Ray ray, const glm::mat4& transform)
	{
		float distance;
		if (!ray.IntersectsAABB(m_BoundingBox, distance))
			return -1;
		
		float distanceOfClosestSubMesh = INT32_MAX;
		int indexOfClosestSubMesh = -1;

		for (int i = 0; i < m_SubMeshes.size(); i++)
		{
			const auto& subMesh = m_SubMeshes[i];

			AABB boundingBox = subMesh.BoundingBox;
			boundingBox.Min = subMesh.WorldTransform * glm::vec4(boundingBox.Min, 1.0f);
			boundingBox.Max = subMesh.WorldTransform * glm::vec4(boundingBox.Max, 1.0f);

			bool intersects = ray.IntersectsAABB(boundingBox, distance);

			if (intersects)
			{
				Ray localRay = ray;
				localRay.Origin = glm::inverse(subMesh.WorldTransform) * glm::vec4(localRay.Origin, 1.0f);
				localRay.Direction = glm::inverse(glm::mat3(subMesh.WorldTransform)) * localRay.Direction;

				for (uint32_t j = subMesh.TriangleOffset; j < (subMesh.TriangleOffset + subMesh.TriangleCount); j++)
				{
					Triangle triangle = m_Triangles[j];
					
					float t = localRay.IntersectsTriangle(triangle);
					
					if (t > -1.0f)
					{
						if (t < distanceOfClosestSubMesh)
						{
							distanceOfClosestSubMesh = t;
							indexOfClosestSubMesh = i;
						}
					}
				}
			}
		}

		if (indexOfClosestSubMesh > -1)
		{
			return indexOfClosestSubMesh;
		}

		return -1;
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