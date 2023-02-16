#pragma once
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	struct BufferInfo
	{
		VkBuffer Buffer = nullptr;
		VmaAllocation Allocation = nullptr;
	};

	// TODO: Add flags to augment buffer usage and memory usage
	// NOTE: All buffers are set to VK_SHARING_MODE_EXCLUSIVE except StorageBuffer as it is needed on both the graphics and compute queue

	class VertexBuffer
	{
	public:
		VertexBuffer(void* data, uint32_t size, const std::string& debugName = "VertexBuffer");
		~VertexBuffer();

	public:
		VkBuffer GetBuffer() { return m_BufferInfo.Buffer; }

		template<typename T>
		T* Map()
		{
			VulkanAllocator allocator(m_DebugName);
			return (T*)allocator.MapMemory<T>(m_BufferInfo.Allocation);
		}

		void Unmap()
		{
			VulkanAllocator allocator(m_DebugName);
			return allocator.UnmapMemory(m_BufferInfo.Allocation);
		}
		
	private:
		BufferInfo m_BufferInfo;
		const std::string m_DebugName;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(void* data, uint32_t size, uint32_t count, const std::string& debugName = "IndexBuffer");
		~IndexBuffer();

	public:
		VkBuffer GetBuffer() { return m_BufferInfo.Buffer; }
		uint32_t GetCount() { return m_Count; }

		template<typename T>
		T* Map()
		{
			VulkanAllocator allocator(m_DebugName);
			return (T*)allocator.MapMemory<T>(m_BufferInfo.Allocation);
		}

		void Unmap()
		{
			VulkanAllocator allocator(m_DebugName);
			return allocator.UnmapMemory(m_BufferInfo.Allocation);
		}

	private:
		BufferInfo m_BufferInfo;
		uint32_t m_Count = 0;
		const std::string m_DebugName;
	};

	class StagingBuffer
	{
	public:
		StagingBuffer(void* data, uint32_t size, const std::string& debugName = "StagingBuffer");
		~StagingBuffer();

	public:
		VkBuffer GetBuffer() { return m_BufferInfo.Buffer; }

		template<typename T>
		T* Map()
		{
			VulkanAllocator allocator(m_DebugName);
			return (T*)allocator.MapMemory<T>(m_BufferInfo.Allocation);
		}

		void Unmap()
		{
			VulkanAllocator allocator(m_DebugName);
			return allocator.UnmapMemory(m_BufferInfo.Allocation);
		}

	private:
		BufferInfo m_BufferInfo;
		const std::string m_DebugName;
	};

	class UniformBuffer
	{
	public:
		UniformBuffer(void* data, uint32_t size, const std::string& debugName = "UniformBuffer");
		~UniformBuffer();

	public:
		VkBuffer GetBuffer() { return m_BufferInfo.Buffer; }
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() { return m_DescriptorBufferInfo; }

		template<typename T>
		T* Map()
		{
			VulkanAllocator allocator(m_DebugName);
			return (T*)allocator.MapMemory<T>(m_BufferInfo.Allocation);
		}

		void Unmap()
		{
			VulkanAllocator allocator(m_DebugName);
			return allocator.UnmapMemory(m_BufferInfo.Allocation);
		}

	private:
		BufferInfo m_BufferInfo;
		VkDescriptorBufferInfo m_DescriptorBufferInfo;
		const std::string m_DebugName;
	};

	class StorageBuffer
	{
	public:
		StorageBuffer(void* data, uint32_t size, const std::string& debugName = "StorageBuffer");
		~StorageBuffer();

	public:
		VkBuffer GetBuffer() { return m_BufferInfo.Buffer; }
	
		template<typename T>
		T* Map()
		{
			VulkanAllocator allocator(m_DebugName);
			return (T*)allocator.MapMemory<T>(m_BufferInfo.Allocation);
		}

		void Unmap()
		{
			VulkanAllocator allocator(m_DebugName);
			return allocator.UnmapMemory(m_BufferInfo.Allocation);
		}

	private:
		BufferInfo m_BufferInfo;
		VkDescriptorBufferInfo m_DescriptorBufferInfo;
		const std::string m_DebugName;
	};

}