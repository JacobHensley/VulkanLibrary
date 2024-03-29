#pragma once
#include "Core/Core.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)																								\
{																														\
	VkResult vk_error_result = (f);																						\
	ASSERT(vk_error_result == VK_SUCCESS, "Vulkan Error: VK_" + VkLibrary::VkTools::ErrorString(vk_error_result));	\
}

namespace VkLibrary {

	namespace VkTools {

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

		// Returns an error code as a string
		std::string ErrorString(VkResult errorCode);

		bool IsDepthFormat(VkFormat format);
		bool IsStencilFormat(VkFormat format);
		VkDescriptorPool CreateDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes = std::vector<VkDescriptorPoolSize>());

		VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, const VkDescriptorSetLayout* layouts, uint32_t count = 1);

		VkWriteDescriptorSet WriteDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			const VkDescriptorBufferInfo* bufferInfo,
			uint32_t descriptorCount = 1);

		VkWriteDescriptorSet WriteDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			const VkDescriptorImageInfo* imageInfo,
			uint32_t descriptorCount = 1);

		void InsertImageMemoryBarrier(
			VkCommandBuffer commandBuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange);

		void SetImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		void SetImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(
			VkDescriptorType type, 
			VkShaderStageFlags stageFlags, 
			uint32_t binding, 
			uint32_t descriptorCount = 1);

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
		void SetImageViewName(VkImageView image, const char* name);
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