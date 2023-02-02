#include "pch.h"
#include "VulkanBuffers.h"
#include "VulkanTools.h"

namespace VkLibrary {

    VertexBuffer::VertexBuffer(void* data, uint32_t size, const std::string& debugName)
        : m_DebugName(debugName)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VulkanAllocator allocator(m_DebugName);
        m_BufferInfo.Allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

        VkTools::SetBufferName(m_BufferInfo.Buffer, m_DebugName.c_str());

        if (data)
        {
            void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
            memcpy(dstBuffer, data, size);
            allocator.UnmapMemory(m_BufferInfo.Allocation);
        }
    }

    VertexBuffer::~VertexBuffer()
    {
        VulkanAllocator allocator(m_DebugName);
        allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
    }

    IndexBuffer::IndexBuffer(void* data, uint32_t size, uint32_t count, const std::string& debugName)
        : m_Count(count), m_DebugName(debugName)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VulkanAllocator allocator(m_DebugName);
        m_BufferInfo.Allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

        VkTools::SetBufferName(m_BufferInfo.Buffer, m_DebugName.c_str());

        if (data)
        {
            void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
            memcpy(dstBuffer, data, size);
            allocator.UnmapMemory(m_BufferInfo.Allocation);
        }
    }

    IndexBuffer::~IndexBuffer()
    {
        VulkanAllocator allocator(m_DebugName);
        allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
    }

    StagingBuffer::StagingBuffer(void* data, uint32_t size, const std::string& debugName)
        : m_DebugName(debugName)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VulkanAllocator allocator(m_DebugName);
        m_BufferInfo.Allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, m_BufferInfo.Buffer);

        VkTools::SetBufferName(m_BufferInfo.Buffer, m_DebugName.c_str());

        if (data)
        {
            void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
            memcpy(dstBuffer, data, size);
            allocator.UnmapMemory(m_BufferInfo.Allocation);
        }
    }

    StagingBuffer::~StagingBuffer()
    {
        VulkanAllocator allocator(m_DebugName);
        allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
    }

    UniformBuffer::UniformBuffer(void* data, uint32_t size, const std::string& debugName)
        : m_DebugName(debugName)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VulkanAllocator allocator(m_DebugName);
        m_BufferInfo.Allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

        VkTools::SetBufferName(m_BufferInfo.Buffer, m_DebugName.c_str());

        m_DescriptorBufferInfo.buffer = m_BufferInfo.Buffer;
        m_DescriptorBufferInfo.offset = 0;
        m_DescriptorBufferInfo.range = size;

        if (data)
        {
            void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
            memcpy(dstBuffer, data, size);
            allocator.UnmapMemory(m_BufferInfo.Allocation);
        }
    }

    UniformBuffer::~UniformBuffer()
    {
        VulkanAllocator allocator(m_DebugName);
        allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
    }

    StorageBuffer::StorageBuffer(void* data, uint32_t size, const std::string& debugName)
        : m_DebugName(debugName)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

        VulkanAllocator allocator(m_DebugName);
        m_BufferInfo.Allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

        VkTools::SetBufferName(m_BufferInfo.Buffer, m_DebugName.c_str());

        m_DescriptorBufferInfo.buffer = m_BufferInfo.Buffer;
        m_DescriptorBufferInfo.offset = 0;
        m_DescriptorBufferInfo.range = size;

        if (data)
        {
            void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
            memcpy(dstBuffer, data, size);
            allocator.UnmapMemory(m_BufferInfo.Allocation);
        }
    }

    StorageBuffer::~StorageBuffer()
    {
        VulkanAllocator allocator(m_DebugName);
        allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
    }

}