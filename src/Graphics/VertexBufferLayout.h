#pragma once
#include "Shader.h"

namespace VkLibrary {

	struct BufferElement
	{
		ShaderDescriptorType Type;
		uint32_t Offset;
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout(const std::map<uint32_t, ShaderAttributeDescription>& attributes, uint32_t bufferBinding = 0);
		VertexBufferLayout(const std::initializer_list<BufferElement>& elements, uint32_t bufferBinding = 0);

	public:
		inline const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributes() const { return m_VertexInputAttributes; }
		inline uint32_t GetStride() const { return m_Stride; }
		inline void SetStride(uint32_t stride) { m_Stride = stride; }

	private:
		std::map<uint32_t, ShaderAttributeDescription> m_Attributes;
		std::vector<BufferElement> m_Elements;

		std::vector<VkVertexInputAttributeDescription> m_VertexInputAttributes;
		uint32_t m_Stride = 0;
		uint32_t m_BufferBinding = 0;
	};

}