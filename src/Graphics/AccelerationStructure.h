#pragma once
#include "Mesh.h"
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct AccelerationStructureSpecification
	{
		Ref<Mesh> Mesh;
		glm::mat4 Transform;
	};

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

	class AccelerationStructure
	{
	public:
		AccelerationStructure(const AccelerationStructureSpecification& specification);
		~AccelerationStructure();

		const VkAccelerationStructureKHR& GetAccelerationStructure() { return m_TopLevelAccelerationStructure.AccelerationStructure; }
		Ref<StorageBuffer> GetSubmeshDataStorageBuffer() const { return m_SubmeshDataStorageBuffer; }

		Ref<StorageBuffer> GetMaterialBuffer() const { return m_MaterialDataStorageBuffer; }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }

		const AccelerationStructureSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Init();

		void CreateTopLevelAccelerationStructure();
		void CreateBottomLevelAccelerationStructure(Ref<Mesh> mesh, const SubMesh& submesh, VulkanAccelerationStructureInfo& outInfo);

	private:
		AccelerationStructureSpecification m_Specification;

		VulkanAccelerationStructureInfo m_TopLevelAccelerationStructure;
		std::vector<VulkanAccelerationStructureInfo> m_BottomLevelAccelerationStructure;

		Ref<StorageBuffer> m_MaterialDataStorageBuffer;
		Ref<StorageBuffer> m_SubmeshDataStorageBuffer;
		std::vector<SubmeshData> m_SubmeshData;
		
		std::vector<MaterialBuffer> m_MaterialData;
		std::vector<Ref<Texture2D>> m_Textures;

		uint32_t m_MaterialIndexOffset = 0;
		uint32_t m_TextureIndexOffset = 0;
	};

}