#pragma once
#include "pch.h"
#include "Core/Core.h"
#include <vulkan/vulkan.h>

namespace VkLibrary {

	enum class ShaderStage
	{
		NONE = -1, VERTEX, FRAGMENT, COMPUTE, RAYGEN, ANY_HIT, CLOSEST_HIT, MISS
	};

	enum class ShaderUniformType
	{
		NONE = -1, BOOL, INT, UINT, FLOAT, FLOAT2, FLOAT3, FLOAT4, MAT4, TEXTURE_2D, TEXTURE_CUBE, IMAGE_2D, IMAGE_CUBE
	};

	struct ShaderUniformDescription
	{
		std::string Name;
		ShaderUniformType Type;
		uint32_t Size;
		uint32_t Offset;
	};

	struct ShaderAttributeDescription
	{
		std::string Name;
		ShaderUniformType Type;
		uint32_t Size;
		uint32_t Offset;
		uint32_t Location;
	};

	struct UniformBufferDescription
	{
		std::string Name;
		uint32_t Size;
		uint32_t BindingPoint;
		uint32_t DescriptorSetID;

		std::vector<ShaderUniformDescription> Uniforms;
	};

	struct StorageBufferDescription
	{
		std::string Name;
		uint32_t BindingPoint;
		uint32_t DescriptorSetID;
	};

	struct ShaderResourceDescription
	{
		std::string Name;
		ShaderUniformType Type;
		uint32_t BindingPoint;
		uint32_t DescriptorSetID;
		uint32_t Dimension;
	};

	struct AccelerationStructureDescription
	{
		std::string Name;
		uint32_t BindingPoint;
		uint32_t DescriptorSetID;
	};

	struct PushConstantRangeDescription
	{
		VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		uint32_t Size = 0;
		uint32_t Offset = 0;
	};

	// TODO: Add debug name support
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

		inline const Ref<VertexBufferLayout>& GetVertexBufferLayout() const { return m_VertexBufferLayout; }

		inline const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderCreateInfo() const { return m_ShaderStageCreateInfo; }

		inline const std::vector<UniformBufferDescription>& GetUniformBufferDescriptions() const { return m_UniformBufferDescriptions; }
		inline const std::vector<StorageBufferDescription>& GetStorageBufferDescriptions() const { return m_StorageBufferDescriptions; }
		inline const std::vector<ShaderResourceDescription>& GetShaderResourceDescriptions() const { return m_ShaderResourceDescriptions; }
		inline const std::vector<ShaderAttributeDescription>& GetShaderAttributeDescriptions() const { return m_ShaderAttributeDescriptions; }
		inline const std::vector<PushConstantRangeDescription>& GetPushConstantRanges() const { return m_PushConstantBufferRanges; }
		inline const std::vector<AccelerationStructureDescription>& GetAccelerationStructureDescriptions() const { return m_AccelerationStructureDescriptions; }

		static uint32_t GetTypeSize(ShaderUniformType type);

	private:
		void Init();

		bool CompileGLSLShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc);
		bool CompileHLSLShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc);
		void ReflectShader(const std::vector<uint32_t>& data, ShaderStage stage);
		void CreateDescriptorSetLayouts();
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
		
		std::vector<UniformBufferDescription> m_UniformBufferDescriptions;
		std::vector<StorageBufferDescription> m_StorageBufferDescriptions;
		std::vector<ShaderResourceDescription> m_ShaderResourceDescriptions;
		std::vector<ShaderAttributeDescription> m_ShaderAttributeDescriptions;
		std::vector<PushConstantRangeDescription> m_PushConstantBufferRanges;
		std::vector<AccelerationStructureDescription> m_AccelerationStructureDescriptions;
	};

}