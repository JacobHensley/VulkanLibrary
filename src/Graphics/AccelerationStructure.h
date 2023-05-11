#pragma once
#include "Mesh.h"
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct VulkanAccelerationStructureInfo
	{
		VkAccelerationStructureKHR AccelerationStructure = nullptr;
		VkDeviceAddress DeviceAddress = 0;
		VkBuffer ASBuffer = nullptr;
		VmaAllocation ASMemory = nullptr;
		VkBuffer ScratchBuffer = nullptr;
		VmaAllocation ScratchMemory = nullptr;
		VkBuffer InstancesBuffer = nullptr;
		VmaAllocation InstancesMemory = nullptr;
		VkBuffer InstancesUploadBuffer = nullptr;
		VmaAllocation InstancesUploadMemory = nullptr;
	};

	struct SubmeshData
	{
		uint32_t BufferIndex;
		uint32_t VertexOffset;
		uint32_t IndexOffset;
		uint32_t MaterialIndex;
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

	struct AccelerationStructureSpecification
	{
		Ref<Mesh> Mesh;
		glm::mat4 Transform;
	};

	// TODO: Support multiple meshes

	class AccelerationStructure
	{
	public:
		AccelerationStructure(const AccelerationStructureSpecification& specification);
		~AccelerationStructure();

	public:
		const AccelerationStructureSpecification& GetSpecification() const { return m_Specification; }
		const VkAccelerationStructureKHR& GetAccelerationStructure() { return m_TopLevelAccelerationStructure.AccelerationStructure; }

	private:
		void Init();

		void CreateTopLevelAccelerationStructure();
		void CreateBottomLevelAccelerationStructure(Ref<Mesh> mesh, const SubMesh& submesh, VulkanAccelerationStructureInfo& outInfo);

	private:
		AccelerationStructureSpecification m_Specification;

		VulkanAccelerationStructureInfo m_TopLevelAccelerationStructure;
		std::vector<VulkanAccelerationStructureInfo> m_BottomLevelAccelerationStructure;

		std::vector<SubmeshData> m_SubmeshData;
		Ref<StorageBuffer> m_SubmeshDataStorageBuffer;

		Ref<StorageBuffer> m_MaterialDataStorageBuffer;
		std::vector<MaterialBuffer> m_MaterialData;
		std::vector<Ref<Texture2D>> m_Textures;

		uint32_t m_MaterialIndexOffset = 0;
		uint32_t m_TextureIndexOffset = 0;
	};

}