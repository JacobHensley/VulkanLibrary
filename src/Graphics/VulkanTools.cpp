#include "pch.h"
#include "VulkanTools.h"
#include "Core/Application.h"

namespace VkLibrary {

	namespace VkTools {

		static VkDebugUtilsMessengerEXT s_DebugUtilsMessenger;

		VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				LOG_ERROR("Validation layer: {0}", pCallbackData->pMessage);

			return VK_FALSE;
		}

		std::string ErrorString(VkResult errorCode)
		{
			switch (errorCode)
			{
	#define STR(r) case VK_ ##r: return #r
				STR(NOT_READY);
				STR(TIMEOUT);
				STR(EVENT_SET);
				STR(EVENT_RESET);
				STR(INCOMPLETE);
				STR(ERROR_OUT_OF_HOST_MEMORY);
				STR(ERROR_OUT_OF_DEVICE_MEMORY);
				STR(ERROR_INITIALIZATION_FAILED);
				STR(ERROR_DEVICE_LOST);
				STR(ERROR_MEMORY_MAP_FAILED);
				STR(ERROR_LAYER_NOT_PRESENT);
				STR(ERROR_EXTENSION_NOT_PRESENT);
				STR(ERROR_FEATURE_NOT_PRESENT);
				STR(ERROR_INCOMPATIBLE_DRIVER);
				STR(ERROR_TOO_MANY_OBJECTS);
				STR(ERROR_FORMAT_NOT_SUPPORTED);
				STR(ERROR_SURFACE_LOST_KHR);
				STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STR(SUBOPTIMAL_KHR);
				STR(ERROR_OUT_OF_DATE_KHR);
				STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STR(ERROR_VALIDATION_FAILED_EXT);
				STR(ERROR_INVALID_SHADER_NV);
	#undef STR
			default:
				return "UNKNOWN_ERROR";
			}
		}

		bool IsDepthFormat(VkFormat format)
		{
			std::vector<VkFormat> formats =
			{
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_X8_D24_UNORM_PACK32,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};

			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		bool IsStencilFormat(VkFormat format)
		{
			std::vector<VkFormat> formats =
			{
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};

			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, const VkDescriptorSetLayout* layouts, uint32_t count)
		{
			Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pSetLayouts = layouts;
			descriptorSetAllocateInfo.descriptorSetCount = count;
			descriptorSetAllocateInfo.descriptorPool = pool;

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device->GetLogicalDevice(), &descriptorSetAllocateInfo, &descriptorSet));
			return descriptorSet;
		}

		VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, uint32_t descriptorCount)
		{
			VkWriteDescriptorSet writeDescriptorSet{};

			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pBufferInfo = bufferInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;

			return writeDescriptorSet;
		}

		VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, const VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount)
		{
			VkWriteDescriptorSet writeDescriptorSet{};

			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pImageInfo = imageInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;

			return writeDescriptorSet;
		}

		void InsertImageMemoryBarrier(
			VkCommandBuffer commandBuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange)
		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.srcAccessMask = srcAccessMask;
			imageMemoryBarrier.dstAccessMask = dstAccessMask;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		void SetImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			// Create an image barrier object
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (oldImageLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (newImageLayout)
			{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Put barrier inside setup command buffer
			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		void SetImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = aspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;
			SetImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
		}

		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount)
		{
			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.descriptorType = type;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorCount = descriptorCount;

			return setLayoutBinding;
		}

		void SetupDebugging(VkInstance instance)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
			debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			debugUtilsMessengerCI.pfnUserCallback = DebugUtilsMessengerCallback;

			VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr, &s_DebugUtilsMessenger);
			assert(result == VK_SUCCESS);
		}

		void FreeDebugCallback(VkInstance instance)
		{
			if (s_DebugUtilsMessenger != VK_NULL_HANDLE)
			{
				vkDestroyDebugUtilsMessengerEXT(instance, s_DebugUtilsMessenger, nullptr);
			}
		}

		void SetObjectName(uint64_t handle, VkObjectType objectType, const char* name)
		{
			VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

			VkDebugUtilsObjectNameInfoEXT objectNameInfo{};
			objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			objectNameInfo.objectHandle = (uint64_t)handle;
			objectNameInfo.pObjectName = name;
			objectNameInfo.objectType = objectType;

			vkSetDebugUtilsObjectNameEXT(device, &objectNameInfo);
		}

		void SetObjectTag(uint64_t handle, VkObjectType objectType, uint64_t name, size_t tagSize, const void* tag)
		{
			VkDevice device = Application::GetVulkanDevice()->GetLogicalDevice();

			VkDebugUtilsObjectTagInfoEXT tagInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT };
			tagInfo.objectType = objectType;
			tagInfo.objectHandle = handle;
			tagInfo.tagName = 0;
			tagInfo.tagSize = tagSize;
			tagInfo.pTag = tag;

			vkSetDebugUtilsObjectTagEXT(device, &tagInfo);
		}

		void SetCommandBufferName(VkCommandBuffer cmdBuffer, const char* name)
		{
			SetObjectName((uint64_t)cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
		}

		void SetQueueName(VkQueue queue, const char* name)
		{
			SetObjectName((uint64_t)queue, VK_OBJECT_TYPE_QUEUE, name);
		}

		void SetImageName(VkImage image, const char* name)
		{
			SetObjectName((uint64_t)image, VK_OBJECT_TYPE_IMAGE, name);
		}

		void SetImageViewName(VkImageView image, const char* name)
		{
			SetObjectName((uint64_t)image, VK_OBJECT_TYPE_IMAGE_VIEW, name);
		}

		void SetSamplerName(VkSampler sampler, const char* name)
		{
			SetObjectName((uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER, name);
		}

		void SetBufferName(VkBuffer buffer, const char* name)
		{
			SetObjectName((uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, name);
		}

		void SetDeviceMemoryName(VkDeviceMemory memory, const char* name)
		{
			SetObjectName((uint64_t)memory, VK_OBJECT_TYPE_DEVICE_MEMORY, name);
		}

		void SetShaderModuleName(VkShaderModule shaderModule, const char* name)
		{
			SetObjectName((uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, name);
		}

		void SetPipelineName(VkPipeline pipeline, const char* name)
		{
			SetObjectName((uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, name);
		}

		void SetPipelineLayoutName(VkPipelineLayout pipelineLayout, const char* name)
		{
			SetObjectName((uint64_t)pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, name);
		}

		void SetRenderPassName(VkRenderPass renderPass, const char* name)
		{
			SetObjectName((uint64_t)renderPass, VK_OBJECT_TYPE_RENDER_PASS, name);
		}

		void SetFramebufferName(VkFramebuffer framebuffer, const char* name)
		{
			SetObjectName((uint64_t)framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, name);
		}

		void SetDescriptorSetLayoutName(VkDescriptorSetLayout descriptorSetLayout, const char* name)
		{
			SetObjectName((uint64_t)descriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, name);
		}

		void SetDescriptorSetName(VkDescriptorSet descriptorSet, const char* name)
		{
			SetObjectName((uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, name);
		}

		void SetSemaphoreName(VkSemaphore semaphore, const char* name)
		{
			SetObjectName((uint64_t)semaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
		}

		void SetFenceName(VkFence fence, const char* name)
		{
			SetObjectName((uint64_t)fence, VK_OBJECT_TYPE_FENCE, name);
		}

		void SetEventName(VkEvent _event, const char* name)
		{
			SetObjectName((uint64_t)_event, VK_OBJECT_TYPE_EVENT, name);
		}

	}

}