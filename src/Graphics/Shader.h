#pragma once
#include "pch.h"
#include "Core/Core.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	enum class ShaderStage
	{
		NONE = -1, VERTEX, FRAGMENT, COMPUTE, RAYGEN, ANY_HIT, CLOSEST_HIT, MISS
	};

	enum class ShaderDescriptorType
	{
		NONE = -1, 
		BOOL, INT, UINT, FLOAT, FLOAT2, FLOAT3, FLOAT4, MAT4,
		TEXTURE_2D, TEXTURE_CUBE, STORAGE_IMAGE_2D, STORAGE_IMAGE_CUBE, ACCELERATION_STRUCTURE,
		UNIFORM_BUFFER, STORAGE_BUFFER
	};

	struct ShaderDescriptor
	{
		std::string Name;
		ShaderDescriptorType Type = ShaderDescriptorType::NONE;
		uint32_t Size = -1;
		uint32_t Offset = -1;
	};

	struct ShaderAttributeDescription
	{
		std::string Name;
		ShaderDescriptorType Type = ShaderDescriptorType::NONE;
		uint32_t Size = -1;
		uint32_t Offset = -1;
		uint32_t Location = -1;
	};

	struct PushConstantRangeDescription
	{
		VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		std::string Name;
		uint32_t Size = -1;
		uint32_t Offset = -1;

		std::vector<ShaderDescriptor> Members;
	};

	struct ShaderResourceDescription
	{
		std::string Name;
		ShaderDescriptorType Type = ShaderDescriptorType::NONE;
		uint32_t Set = -1;
		uint32_t Binding = -1;
	};

	struct ShaderBufferDescription
	{
		std::string Name;
		ShaderDescriptorType Type = ShaderDescriptorType::NONE;
		uint32_t Set = -1;
		uint32_t Binding = -1;
		uint32_t Size = -1;

		std::vector<ShaderDescriptor> Members;
	};

	struct ShaderDescriptorMetadata
	{
		ShaderDescriptorType Type = ShaderDescriptorType::NONE;
		uint32_t Set = -1;
		uint32_t Binding = -1;
	};

	// TODO: Provide support array items in shader reflection and layout generation
	// NOTE: Reflection for HLSL is not entirely accurate

	class VertexBufferLayout;

	class Shader
	{
	public:
		Shader(const std::string_view path);
		Shader(const std::string_view path, const std::string_view entryPoint, const std::vector<std::wstring>& defines = {});
		~Shader();

	public:
		inline const std::filesystem::path& GetPath() const { return m_Path; }
		inline bool CompiledSuccessfully() const { return m_CompilationStatus; }

		const ShaderResourceDescription& FindResourceDescription(const std::string& name);
		const ShaderBufferDescription& FindBufferDescription(const std::string& name);
		const VkWriteDescriptorSet& FindWriteDescriptorSet(const std::string& name);

		VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, uint32_t set);
		const std::map<uint32_t, std::map<uint32_t, VkWriteDescriptorSet>>& GetWriteDescriptorSets() const { return m_WriteDescriptorSets; }

		inline const Ref<VertexBufferLayout>& GetVertexBufferLayout() const { return m_VertexBufferLayout; }
		inline const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderCreateInfo() const { return m_ShaderStageCreateInfo; }

		inline const std::map<uint32_t, std::map<uint32_t, ShaderBufferDescription>>& GetShaderBufferDescriptions() const { return m_ShaderBufferDescriptions; }
		inline const std::map<uint32_t, std::map<uint32_t, ShaderResourceDescription>>& GetShaderResourceDescriptions() const { return m_ShaderResourceDescriptions; }

		inline const std::map<uint32_t, ShaderAttributeDescription>& GetShaderAttributeDescriptions() const { return m_ShaderAttributeDescriptions; }
		inline const std::vector<PushConstantRangeDescription>& GetPushConstantRanges() const { return m_PushConstantBufferRanges; }

		static uint32_t GetTypeSize(ShaderDescriptorType type);

	private:
		void Init();

		bool CompileGLSLShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc);
		bool CompileHLSLShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc);

		void ReflectShader(const std::vector<uint32_t>& data, ShaderStage stage);
		void GenerateDescriptorData();

		std::unordered_map<ShaderStage, std::string> SplitShaders(const std::filesystem::path& path);

	private:
		std::filesystem::path m_Path;
		std::unordered_map<ShaderStage, std::string> m_ShaderSrc;

		bool m_CompilationStatus = false;

		std::vector<std::wstring> m_HLSLDefines;
		const std::string m_HLSLEntryPoint;

		Ref<VertexBufferLayout> m_VertexBufferLayout;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStageCreateInfo;
		std::map<uint32_t, std::map<uint32_t, VkWriteDescriptorSet>> m_WriteDescriptorSets;
		
		// Map of set->binding to resource descriptions
		std::map<uint32_t, std::map<uint32_t, ShaderBufferDescription>> m_ShaderBufferDescriptions;
		std::map<uint32_t, std::map<uint32_t, ShaderResourceDescription>> m_ShaderResourceDescriptions;

		std::map<uint32_t, ShaderAttributeDescription> m_ShaderAttributeDescriptions;
		std::vector<PushConstantRangeDescription> m_PushConstantBufferRanges;

		std::unordered_map<std::string, ShaderDescriptorMetadata> m_ShaderDescriptorMetadata;
	};

}