#include "pch.h"
#include "Shader.h"
#include "VulkanTools.h"
#include "Core/Application.h"
#include "VertexBufferLayout.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_common.hpp>
#include <unknwn.h>
#include <dxc/dxcapi.h>
#include <combaseapi.h>
#include <codecvt>

namespace VkLibrary {

	namespace Utils {

		static shaderc_shader_kind ShaderStageToShaderc(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:	   return shaderc_vertex_shader;
			case ShaderStage::FRAGMENT:    return shaderc_fragment_shader;
			case ShaderStage::COMPUTE:     return shaderc_compute_shader;
			case ShaderStage::RAYGEN:      return shaderc_raygen_shader;
			case ShaderStage::MISS:        return shaderc_miss_shader;
			case ShaderStage::CLOSEST_HIT: return shaderc_closesthit_shader;
			}

			ASSERT(false, "Unknown Type");
			return (shaderc_shader_kind)0;
		}

		static VkShaderStageFlagBits ShaderStageToVulkan(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:	   return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderStage::FRAGMENT:    return VK_SHADER_STAGE_FRAGMENT_BIT;
			case ShaderStage::COMPUTE:     return VK_SHADER_STAGE_COMPUTE_BIT;
			case ShaderStage::RAYGEN:      return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			case ShaderStage::MISS:        return VK_SHADER_STAGE_MISS_BIT_KHR;
			case ShaderStage::CLOSEST_HIT: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			}

			ASSERT(false, "Unknown Type");
			return (VkShaderStageFlagBits)0;
		}

		static const std::string ShaderStageToDXC(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:	   return "vs_6_2";
			case ShaderStage::FRAGMENT:    return "ps_6_2";
			case ShaderStage::COMPUTE:     return "cs_6_2";
			}

			ASSERT(false, "Unsupported HLSL stage");
			return "";
		}

		static const std::string ShaderStageToString(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:	   return "Vertex";
			case ShaderStage::FRAGMENT:    return "Fragment";
			case ShaderStage::COMPUTE:     return "Compute";
			case ShaderStage::RAYGEN:      return "RayGen";
			case ShaderStage::MISS:        return "Miss";
			case ShaderStage::CLOSEST_HIT: return "ClosestHit";
			}

			ASSERT(false, "Unknown Type");
			return "";
		}

		static ShaderUniformType GetType(spirv_cross::SPIRType type)
		{
			spirv_cross::SPIRType::BaseType baseType = type.basetype;

			if (baseType == spirv_cross::SPIRType::Float)
			{
				if (type.columns == 1)
				{
					if (type.vecsize == 1)	    return ShaderUniformType::FLOAT;
					else if (type.vecsize == 2) return ShaderUniformType::FLOAT2;
					else if (type.vecsize == 3) return ShaderUniformType::FLOAT3;
					else if (type.vecsize == 4) return ShaderUniformType::FLOAT4;
				}
				else
				{
					return ShaderUniformType::MAT4;
				}
			}
			else if (baseType == spirv_cross::SPIRType::Image)
			{
				if (type.image.dim == 1)	     return ShaderUniformType::STORAGE_IMAGE_2D;
				else if (type.image.dim == 3)    return ShaderUniformType::STORAGE_IMAGE_CUBE;
			}
			else if (baseType == spirv_cross::SPIRType::SampledImage)
			{
				if (type.image.dim == 1)		 return ShaderUniformType::TEXTURE_2D;
				else if (type.image.dim == 3)    return ShaderUniformType::TEXTURE_CUBE;
			}
			else if (baseType == spirv_cross::SPIRType::Int)
			{
				return ShaderUniformType::INT;
			}
			else if (baseType == spirv_cross::SPIRType::UInt)
			{
				return ShaderUniformType::UINT;
			}
			else if (baseType == spirv_cross::SPIRType::Boolean)
			{
				return ShaderUniformType::BOOL;
			}

			return (ShaderUniformType)0;
		}

		static VkDescriptorType TypeToVkDescriptorType(ShaderUniformType type)
		{
			switch (type)
			{
			case ShaderUniformType::TEXTURE_2D:             return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case ShaderUniformType::TEXTURE_CUBE:           return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case ShaderUniformType::STORAGE_IMAGE_2D:       return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case ShaderUniformType::STORAGE_IMAGE_CUBE:     return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case ShaderUniformType::UNIFORM_BUFFER:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case ShaderUniformType::STORAGE_BUFFER:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case ShaderUniformType::ACCELERATION_STRUCTURE: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			}

			ASSERT(false, "Unknown Type");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}


		static IDxcCompiler3* s_HLSLCompiler;
		static IDxcUtils* s_HLSLUtils;
		static IDxcIncludeHandler* s_DefaultIncludeHandler;

		class DXCIncludeHandler : public IDxcIncludeHandler
		{
		public:
			DXCIncludeHandler()
			{
			}

			HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
			{
				IDxcBlobEncoding* pEncoding;

				std::wstring wstring(pFilename);
				int count = WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), (int)wstring.length(), NULL, 0, NULL, NULL);
				std::string path(count, 0);
				WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, &path[0], count, NULL, NULL);

				if (IncludedFiles.find(path) != IncludedFiles.end())
				{
					// Return empty string blob if this file has been included before
					static const char nullStr[] = " ";
					s_HLSLUtils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, &pEncoding);
					*ppIncludeSource = pEncoding;
					return S_OK;
				}

				HRESULT hr = s_HLSLUtils->LoadFile(pFilename, nullptr, &pEncoding);
				if (SUCCEEDED(hr))
				{
					IncludedFiles.insert(path);
					*ppIncludeSource = pEncoding;
				}
				else
				{
					*ppIncludeSource = nullptr;
				}
				return hr;
			}

			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
			{
				return s_DefaultIncludeHandler->QueryInterface(riid, ppvObject);
			}

			ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
			ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

			std::unordered_set<std::string> IncludedFiles;
		};

		class ShadercIncludeInterface : public shaderc::CompileOptions::IncluderInterface
		{
			shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)
			{
				const std::string name = std::string(requested_source);

				std::ifstream t(name);
				std::stringstream buffer;
				buffer << t.rdbuf();

				const std::string contents = buffer.str();

				auto container = new std::array<std::string, 2>;
				(*container)[0] = name;
				(*container)[1] = contents;

				auto data = new shaderc_include_result;

				data->user_data = container;

				data->source_name = (*container)[0].data();
				data->source_name_length = (*container)[0].size();

				data->content = (*container)[1].data();
				data->content_length = (*container)[1].size();

				return data;
			};

			void ReleaseInclude(shaderc_include_result* data) override
			{
				delete static_cast<std::array<std::string, 2>*>(data->user_data);
				delete data;
			};
		};

	}

	Shader::Shader(const std::string_view path)
		: m_Path(path), m_HLSLEntryPoint("main"), m_HLSLDefines({})
	{
		Init();
	}

	Shader::Shader(const std::string_view path, const std::string_view entryPoint, const std::vector<std::wstring>& defines)
		: m_Path(path), m_HLSLEntryPoint(entryPoint), m_HLSLDefines(defines)
	{
		Init();
	}

	Shader::~Shader()
	{
		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

		for (auto shaderStageInfo : m_ShaderStageCreateInfo)
		{
			vkDestroyShaderModule(device, shaderStageInfo.module, nullptr);
		}

		for (int i = 0; i < m_DescriptorSetLayouts.size(); i++)
		{
			vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts[i], nullptr);
		}
	}

	void Shader::Init()
	{
		if (!Utils::s_HLSLCompiler || !Utils::s_HLSLUtils)
		{
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Utils::s_HLSLCompiler));
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils::s_HLSLUtils));
		}

		m_ShaderSrc = SplitShaders(m_Path);
		ASSERT(m_ShaderSrc.size() >= 1, "Shader is empty or path is invalid");

		if (m_Path.extension() == ".glsl")
		{
			m_CompilationStatus = CompileGLSLShaders(m_ShaderSrc);
		}
		else if (m_Path.extension() == ".hlsl")
		{
			m_CompilationStatus = CompileHLSLShaders(m_ShaderSrc);
		}
		else
		{
			ASSERT(false, "File extension is not supported");
		}

		ASSERT(m_CompilationStatus, "Failed to initialize shader");

		CreateDescriptorSetLayouts();
	}

	bool Shader::CompileGLSLShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc)
	{
		// Setup compiler
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.SetIncluder(std::make_unique<Utils::ShadercIncludeInterface>());

		for (auto&& [stage, src] : m_ShaderSrc)
		{
			// Preprocess includes 
			shaderc::PreprocessedSourceCompilationResult preprocessResult = compiler.PreprocessGlsl(src, Utils::ShaderStageToShaderc(stage), m_Path.string().c_str(), options);
			if (preprocessResult.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				LOG_ERROR("Warnings ({0}), Errors ({1}) \n{2}", preprocessResult.GetNumWarnings(), preprocessResult.GetNumErrors(), preprocessResult.GetErrorMessage());
				return false;
			}

			std::string preprocessSource(preprocessResult.begin());

			// Compile shader source and check for errors
			shaderc::SpvCompilationResult compilationResult = compiler.CompileGlslToSpv(preprocessSource, Utils::ShaderStageToShaderc(stage), m_Path.string().c_str(), options);
			if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				LOG_ERROR("Warnings ({0}), Errors ({1}) \n{2}", compilationResult.GetNumWarnings(), compilationResult.GetNumErrors(), compilationResult.GetErrorMessage());
				return false;
			}

			// Format data
			const uint8_t* data = reinterpret_cast<const uint8_t*>(compilationResult.cbegin());
			const uint8_t* dataEnd = reinterpret_cast<const uint8_t*>(compilationResult.cend());
			uint32_t size = (uint32_t)(dataEnd - data);

			std::vector<uint32_t> spirv(compilationResult.cbegin(), compilationResult.cend());

			VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

			// Create shader module
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = size;
			createInfo.pCode = spirv.data();

			VkShaderModule shaderModule;
			VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

			// Create shader stage
			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = Utils::ShaderStageToVulkan(stage);
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = "main";

			m_ShaderStageCreateInfo.push_back(shaderStageInfo);
			ReflectShader(spirv, stage);
		}

		return true;
	}

	bool Shader::CompileHLSLShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc)
	{
		std::vector<const wchar_t*> arguments;

		// Set target
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-fspv-target-env=vulkan1.2");

		// Strip reflection infomation
		arguments.push_back(L"-Qstrip_debug");
		arguments.push_back(L"-Qstrip_reflect");

		// Set include directory
		const std::string& path = (std::filesystem::current_path() / m_Path.parent_path()).string();
		std::wstring pathW = std::wstring(path.begin(), path.end());
		arguments.push_back(L"-I");
		arguments.push_back(pathW.c_str());

		// Set entry point
		std::wstring entryPointW = std::wstring(m_HLSLEntryPoint.begin(), m_HLSLEntryPoint.end());
		arguments.push_back(L"-E");
		arguments.push_back(entryPointW.c_str());
		
		// Set defines
		for (const std::wstring& define : m_HLSLDefines)
		{
			arguments.push_back(L"-D");
			arguments.push_back(define.c_str());
		}

		arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		arguments.push_back(DXC_ARG_DEBUG);
		arguments.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);

		for (const auto& [stage, shaderSrc] : shaderSrc)
		{
			// Set shader stage
			const std::string DXCstage = Utils::ShaderStageToDXC(stage);
			std::wstring DXCstageW = std::wstring(DXCstage.begin(), DXCstage.end());

			arguments.push_back(L"-T");
			arguments.push_back(DXCstageW.c_str());
			
			IDxcBlobEncoding* blobEncoding;
			Utils::s_HLSLUtils->CreateBlob(shaderSrc.c_str(), (uint32_t)shaderSrc.size(), CP_UTF8, &blobEncoding);

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = blobEncoding->GetBufferPointer();
			sourceBuffer.Size = blobEncoding->GetBufferSize();
			sourceBuffer.Encoding = 0;

			Utils::DXCIncludeHandler includeHandler = Utils::DXCIncludeHandler();

			IDxcResult* compileResult;
			Utils::s_HLSLCompiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), &includeHandler, IID_PPV_ARGS(&compileResult));

			IDxcBlobUtf8* errors;
			compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), 0);
			if (errors && errors->GetStringLength() > 0)
			{
				LOG_ERROR((char*)errors->GetBufferPointer());
				return false;
			}

			arguments.pop_back();
			arguments.pop_back();

			IDxcBlob* pResult;
			compileResult->GetResult(&pResult);

			size_t size = pResult->GetBufferSize();
			std::vector<uint32_t> spirv(size / sizeof(uint32_t));
			std::memcpy(spirv.data(), pResult->GetBufferPointer(), size);

			// Create shader module
			VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = size;
			createInfo.pCode = spirv.data();

			VkShaderModule shaderModule;
			VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = Utils::ShaderStageToVulkan(stage);;
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = m_HLSLEntryPoint.c_str();

			m_ShaderStageCreateInfo.push_back(shaderStageInfo);
			ReflectShader(spirv, stage);
		}

		return true;
	}

	// dstSet and pBufferInfo or pImageInfo is set by the user before use
	VkWriteDescriptorSet Shader::GenerateWriteDescriptor(const std::string& name)
	{
		if (m_ResourceNamesAndTypes.find(name) != m_ResourceNamesAndTypes.end())
		{
			const auto [binding, type] = m_ResourceNamesAndTypes.at(name);

			VkWriteDescriptorSet writeDescriptor = {};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.descriptorType = Utils::TypeToVkDescriptorType(type);
			writeDescriptor.dstBinding = binding;
			writeDescriptor.descriptorCount = 1;

			return writeDescriptor;
		}

		ASSERT(false, "Could not find shader resource name");
		return VkWriteDescriptorSet();
	}

	VkDescriptorSet Shader::AllocateDescriptorSet(VkDescriptorPool pool)
	{
		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pSetLayouts = GetDescriptorSetLayouts().data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)GetDescriptorSetLayouts().size();
		descriptorSetAllocateInfo.descriptorPool = pool;

		VkDescriptorSet descriptorSet;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

		return descriptorSet;
	}

	void Shader::ReflectShader(const std::vector<uint32_t>& data, ShaderStage stage)
	{
		spirv_cross::Compiler compiler(data);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		// Get all push constant ranges
		for (const spirv_cross::Resource& resource : resources.push_constant_buffers)
		{
			uint32_t bufferOffset = 0;

			const auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);

			// Calculate range offest based on last buffers offset and size
			if (m_PushConstantBufferRanges.size())
				bufferOffset = m_PushConstantBufferRanges.back().Offset + m_PushConstantBufferRanges.back().Size;

			auto& pushConstantRange = m_PushConstantBufferRanges.emplace_back();
			pushConstantRange.ShaderStage = Utils::ShaderStageToVulkan(stage);
			pushConstantRange.Size = (uint32_t)(bufferSize - bufferOffset);
			pushConstantRange.Offset = bufferOffset;
		}

		// Get all uniform buffers
		for (const spirv_cross::Resource& resource : resources.uniform_buffers)
		{
			auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t memberCount = bufferType.member_types.size();

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			if (m_UniformBufferDescriptions.find(binding) != m_UniformBufferDescriptions.end())
			{
				LOG_WARN("Binding {} already exists ({})", binding, m_UniformBufferDescriptions[binding].Name);
			}

			UniformBufferDescription& buffer = m_UniformBufferDescriptions[binding];
			buffer.Name = resource.name;
			buffer.Size = (uint32_t)compiler.get_declared_struct_size(bufferType);
			buffer.BindingPoint = binding;
			buffer.DescriptorSetID = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			m_ResourceNamesAndTypes[buffer.Name] = {buffer.BindingPoint, ShaderUniformType::UNIFORM_BUFFER};

			// Get all members of the uniform buffer
			for (int i = 0; i < memberCount; i++)
			{
				ShaderUniformDescription uniform;
				uniform.Name = compiler.get_member_name(bufferType.self, i);
				uniform.Size = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i);
				uniform.Type = Utils::GetType(compiler.get_type(bufferType.member_types[i]));
				uniform.Offset = compiler.type_struct_member_offset(bufferType, i);

				buffer.Uniforms.push_back(uniform);
			}
		}

		// Get all storage buffers
		for (const spirv_cross::Resource& resource : resources.storage_buffers)
		{
			auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t memberCount = bufferType.member_types.size();

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			if (m_StorageBufferDescriptions.find(binding) != m_StorageBufferDescriptions.end())
			{
				LOG_WARN("Binding {} already exists ({})", binding, m_StorageBufferDescriptions[binding].Name);
			}

			StorageBufferDescription& buffer = m_StorageBufferDescriptions[binding];
			buffer.Name = resource.name;
			buffer.BindingPoint = binding;
			buffer.DescriptorSetID = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			m_ResourceNamesAndTypes[buffer.Name] = { buffer.BindingPoint, ShaderUniformType::STORAGE_BUFFER };
		}

		// Get all vertex attributes
		if (stage == ShaderStage::VERTEX)
		{
			uint32_t offset = 0;
			for (const spirv_cross::Resource& resource : resources.stage_inputs)
			{
				auto& type = compiler.get_type(resource.base_type_id);

				uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
				ShaderAttributeDescription& attribute = m_ShaderAttributeDescriptions[location];
				attribute.Name = resource.name;
				attribute.Location = location;
				attribute.Type = Utils::GetType(type);
				attribute.Size = GetTypeSize(attribute.Type);
				attribute.Offset = offset;

				offset += attribute.Size;
			}

			m_VertexBufferLayout = CreateRef<VertexBufferLayout>(m_ShaderAttributeDescriptions);
		}

		// Get all sampled images in the shader
		for (auto& resource : resources.sampled_images)
		{
			auto& type = compiler.get_type(resource.base_type_id);

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			if (m_ShaderResourceDescriptions.find(binding) != m_ShaderResourceDescriptions.end())
			{
				LOG_WARN("Binding {} already exists ({})", binding, m_ShaderResourceDescriptions[binding].Name);
			}
		
			ShaderResourceDescription& shaderResource = m_ShaderResourceDescriptions[binding];
			shaderResource.Name = resource.name;
			shaderResource.BindingPoint = binding;
			shaderResource.DescriptorSetID = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			shaderResource.Dimension = type.image.dim;
			shaderResource.Type = Utils::GetType(type);

			m_ResourceNamesAndTypes[shaderResource.Name] = { shaderResource.BindingPoint, shaderResource.Type };
		}

		// Get all storage images
		for (const spirv_cross::Resource& resource : resources.storage_images)
		{
			auto& type = compiler.get_type(resource.base_type_id);

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			if (m_ShaderResourceDescriptions.find(binding) != m_ShaderResourceDescriptions.end())
			{
				LOG_WARN("Binding {} already exists ({})", binding, m_ShaderResourceDescriptions[binding].Name);
			}

			ShaderResourceDescription& shaderResource = m_ShaderResourceDescriptions[binding];
			shaderResource.Name = resource.name;
			shaderResource.BindingPoint = binding;
			shaderResource.DescriptorSetID = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			shaderResource.Dimension = type.image.dim;
			shaderResource.Type = Utils::GetType(type);

			m_ResourceNamesAndTypes[shaderResource.Name] = { shaderResource.BindingPoint, shaderResource.Type };
		}

		// Get all acceleration structures
		for (const spirv_cross::Resource& resource : resources.acceleration_structures)
		{
			auto& type = compiler.get_type(resource.base_type_id);

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			if (m_ShaderResourceDescriptions.find(binding) != m_ShaderResourceDescriptions.end())
			{
				LOG_WARN("Binding {} already exists ({})", binding, m_ShaderResourceDescriptions[binding].Name);
			}

			ShaderResourceDescription& shaderResource = m_ShaderResourceDescriptions[binding];
			shaderResource.Name = resource.name;
			shaderResource.BindingPoint = binding;
			shaderResource.DescriptorSetID = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			m_ResourceNamesAndTypes[shaderResource.Name] = { shaderResource.BindingPoint, shaderResource.Type };
		}
	}

	void Shader::CreateDescriptorSetLayouts()
	{
		VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

		std::unordered_map<int, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;

		// Create uniform buffer layout bindings
		for (const auto& [binding, uniformBufferDescriptions] : m_UniformBufferDescriptions)
		{
			VkDescriptorSetLayoutBinding layout{};

			layout.binding = uniformBufferDescriptions.BindingPoint;
			layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layout.descriptorCount = 1;
			layout.stageFlags = VK_SHADER_STAGE_ALL;
			layout.pImmutableSamplers = nullptr;

			descriptorSetLayoutBindings[uniformBufferDescriptions.DescriptorSetID].push_back(layout);
		}

		// Create storage buffer layout bindings
		for (const auto& [binding, storageBufferDescriptions] : m_StorageBufferDescriptions)
		{
			VkDescriptorSetLayoutBinding layout{};

			layout.binding = storageBufferDescriptions.BindingPoint;
			layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layout.descriptorCount = 1;
			layout.stageFlags = VK_SHADER_STAGE_ALL;
			layout.pImmutableSamplers = nullptr;

			descriptorSetLayoutBindings[storageBufferDescriptions.DescriptorSetID].push_back(layout);
		}

		// Create acceleration structures layout bindings
		for (const auto& [binding, accelerationStructureDescription] : m_AccelerationStructureDescriptions)
		{
			VkDescriptorSetLayoutBinding layout{};

			layout.binding = accelerationStructureDescription.BindingPoint;
			layout.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			layout.descriptorCount = 1;
			layout.stageFlags = VK_SHADER_STAGE_ALL;
			layout.pImmutableSamplers = nullptr;

			descriptorSetLayoutBindings[accelerationStructureDescription.DescriptorSetID].push_back(layout);
		}

		// Create resource layout bindings
		for (const auto&[binding, resourceDescription] : m_ShaderResourceDescriptions)
		{
			VkDescriptorSetLayoutBinding layout{};

			layout.binding = resourceDescription.BindingPoint;
			layout.descriptorCount = 1;
			layout.stageFlags = VK_SHADER_STAGE_ALL;
			layout.pImmutableSamplers = nullptr;

			ShaderUniformType type = resourceDescription.Type;
			if (type == ShaderUniformType::STORAGE_IMAGE_2D || type == ShaderUniformType::STORAGE_IMAGE_CUBE)
				layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			else
				layout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			descriptorSetLayoutBindings[resourceDescription.DescriptorSetID].push_back(layout);
		}

		// Use layout bindings to create descriptor set layouts
		for (const auto& [DescriptorSetID, bindings] : descriptorSetLayoutBindings)
		{
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = (uint32_t)bindings.size();
			layoutInfo.pBindings = bindings.data();

			if (DescriptorSetID >= m_DescriptorSetLayouts.size())
				m_DescriptorSetLayouts.resize(DescriptorSetID + 1);

			VkDescriptorSetLayout& descriptorSetLayout = m_DescriptorSetLayouts[DescriptorSetID];
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));
		}

		// Fill empty slots in m_DescriptorSetLayouts with empty descriptor set layouts
		for (auto& dsl : m_DescriptorSetLayouts)
		{
			if (!dsl)
			{
				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = 0;
				layoutInfo.pBindings = nullptr;
				VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &dsl));
			}
		}
	}

	std::unordered_map<ShaderStage, std::string> Shader::SplitShaders(const std::filesystem::path& path)
	{
		std::unordered_map<ShaderStage, std::string> result;
		ShaderStage stage = ShaderStage::NONE;

		std::ifstream stream(path);
		ASSERT(stream.good(), "File either does exist or is empty");

		std::stringstream ss[2];
		std::string line;

		while (getline(stream, line))
		{
			if (line.find("#Shader") != std::string::npos)
			{
				if (line.find("Vertex") != std::string::npos)
				{
					stage = ShaderStage::VERTEX;
				}
				else if (line.find("Fragment") != std::string::npos)
				{
					stage = ShaderStage::FRAGMENT;
				}
				else if (line.find("Compute") != std::string::npos)
				{
					stage = ShaderStage::COMPUTE;
				}
				else if (line.find("RayGen") != std::string::npos)
				{
					stage = ShaderStage::RAYGEN;
				}
				else if (line.find("AnyHit") != std::string::npos)
				{
					stage = ShaderStage::ANY_HIT;
				}
				else if (line.find("ClosestHit") != std::string::npos)
				{
					stage = ShaderStage::CLOSEST_HIT;
				}
				else if (line.find("Miss") != std::string::npos)
				{
					stage = ShaderStage::MISS;
				}
			}
			else
			{
				result[stage] += line + '\n';
			}
		}

		return result;
	}

	uint32_t Shader::GetTypeSize(ShaderUniformType type)
	{
		switch (type)
		{
		case ShaderUniformType::BOOL:   return 4;
		case ShaderUniformType::INT:    return 4;
		case ShaderUniformType::UINT:   return 4;
		case ShaderUniformType::FLOAT:  return 4;
		case ShaderUniformType::FLOAT2: return 4 * 2;
		case ShaderUniformType::FLOAT3: return 4 * 3;
		case ShaderUniformType::FLOAT4: return 4 * 4;
		case ShaderUniformType::MAT4:   return 64;
		}

		ASSERT(false, "Unknown Type");
		return 0;
	}

}