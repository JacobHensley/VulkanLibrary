#pragma once
#include "Core/Core.h"
#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

#define VK_CHECK_RESULT(f)																								\
{																														\
	VkResult vk_error_result = (f);																						\
	ASSERT(vk_error_result == VK_SUCCESS, "Vulkan Error: VK_" + VkLibrary::VkTools::ErrorString(vk_error_result));	\
}

namespace VkLibrary {

	namespace VkTools {
		// Returns an error code as a string
		std::string ErrorString(VkResult errorCode);

		// Default validation layers
		extern int validationLayerCount;
		extern const char* validationLayerNames[];

		// Default debug callback
		VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t srcObject,
			size_t location,
			int32_t msgCode,
			const char* pLayerPrefix,
			const char* pMsg,
			void* pUserData);

		// Load debug function pointers and set debug callback
		// if callBack is NULL, default message callback will be used
		void SetupDebugging(VkInstance instance);

		// Clear debug callback
		void FreeDebugCallback(VkInstance instance);

		// Sets the debug name of an object
		// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
		// along with the object type
		void SetObjectName(uint64_t object, VkObjectType objectType, const char* name);

		// Set the tag for an object
		void SetObjectTag(uint64_t object, VkObjectType objectType, uint64_t name, size_t tagSize, const void* tag);

		// Object specific naming functions
		void SetCommandBufferName(VkCommandBuffer cmdBuffer, const char* name);
		void SetQueueName(VkQueue queue, const char* name);
		void SetImageName(VkImage image, const char* name);
		void SetSamplerName(VkSampler sampler, const char* name);
		void SetBufferName(VkBuffer buffer, const char* name);
		void SetDeviceMemoryName(VkDeviceMemory memory, const char* name);
		void SetShaderModuleName(VkShaderModule shaderModule, const char* name);
		void SetPipelineName(VkPipeline pipeline, const char* name);
		void SetPipelineLayoutName(VkPipelineLayout pipelineLayout, const char* name);
		void SetRenderPassName(VkRenderPass renderPass, const char* name);
		void SetFramebufferName(VkFramebuffer framebuffer, const char* name);
		void SetDescriptorSetLayoutName(VkDescriptorSetLayout descriptorSetLayout, const char* name);
		void SetDescriptorSetName(VkDescriptorSet descriptorSet, const char* name);
		void SetSemaphoreName(VkSemaphore semaphore, const char* name);
		void SetFenceName(VkFence fence, const char* name);
		void SetEventName(VkEvent _event, const char* name);

	}

}