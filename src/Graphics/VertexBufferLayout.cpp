#include "pch.h"
#include "VertexBufferLayout.h"

namespace VkLibrary {

	namespace Utils {

		static VkFormat ShaderTypeToVkFormat(ShaderDescriptorType type)
		{
			switch (type)
			{
			case VkLibrary::ShaderDescriptorType::INT:    return VK_FORMAT_R32_SINT;
			case VkLibrary::ShaderDescriptorType::UINT:   return VK_FORMAT_R32_UINT;
			case VkLibrary::ShaderDescriptorType::FLOAT:  return VK_FORMAT_R32_SFLOAT;
			case VkLibrary::ShaderDescriptorType::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
			case VkLibrary::ShaderDescriptorType::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
			case VkLibrary::ShaderDescriptorType::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			ASSERT(false, "Unknown Type");
			return (VkFormat)0;
		}

	}

	VertexBufferLayout::VertexBufferLayout(const std::map<uint32_t, ShaderAttributeDescription>& attributes, uint32_t bufferBinding)
		: m_Attributes(attributes), m_BufferBinding(bufferBinding)
	{
		if (m_Attributes.size() == 0)
		{
			return;
		}
		
		m_VertexInputAttributes.resize(m_Attributes.size());
		for (int i = 0; i < m_Attributes.size(); i++)
		{
			m_VertexInputAttributes[i].binding = 0;
			m_VertexInputAttributes[i].location = m_Attributes[i].Location;
			m_VertexInputAttributes[i].format = Utils::ShaderTypeToVkFormat(m_Attributes[i].Type);
			m_VertexInputAttributes[i].offset = m_Attributes[i].Offset;
		}
		
		m_Stride = m_Attributes.rbegin()->second.Offset + m_Attributes.rbegin()->second.Size;
	}

	VertexBufferLayout::VertexBufferLayout(const std::initializer_list<BufferElement>& elements, uint32_t bufferBinding)
		: m_Elements(elements), m_BufferBinding(bufferBinding)
	{
		if (m_Elements.size() == 0)
		{
			return;
		}

		m_VertexInputAttributes.resize(m_Elements.size());

		for (uint32_t i = 0; i < m_Elements.size(); i++)
		{
			m_VertexInputAttributes[i].binding = 0;
			m_VertexInputAttributes[i].location = i;
			m_VertexInputAttributes[i].format = Utils::ShaderTypeToVkFormat(m_Elements[i].Type);
			m_VertexInputAttributes[i].offset = m_Elements[i].Offset;
		}

		uint32_t size = Shader::GetTypeSize(m_Elements.back().Type);
		m_Stride = m_Elements.back().Offset + size;
	}

}