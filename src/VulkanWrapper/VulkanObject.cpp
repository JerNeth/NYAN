#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/Instance.h"

template<typename VKObject>
VkObjectType get_object_type() {
    return VK_OBJECT_TYPE_UNKNOWN;
}
template<>
VkObjectType get_object_type<VkInstance>() {
    return VK_OBJECT_TYPE_INSTANCE;
}
template<>
VkObjectType get_object_type<VkPhysicalDevice>() {
    return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
}
template<>
VkObjectType get_object_type<VkDevice>() {
    return VK_OBJECT_TYPE_DEVICE;
}
template<>
VkObjectType get_object_type<VkQueue>() {
    return VK_OBJECT_TYPE_QUEUE;
}
template<>
VkObjectType get_object_type<VkSemaphore>() {
    return VK_OBJECT_TYPE_SEMAPHORE;
}
template<>
VkObjectType get_object_type<VkCommandBuffer>() {
    return VK_OBJECT_TYPE_COMMAND_BUFFER;
}
template<>
VkObjectType get_object_type<VkFence>() {
    return VK_OBJECT_TYPE_FENCE;
}
template<>
VkObjectType get_object_type<VkDeviceMemory>() {
    return VK_OBJECT_TYPE_DEVICE_MEMORY;
}
template<>
VkObjectType get_object_type<VkBuffer>() {
    return VK_OBJECT_TYPE_BUFFER;
}
template<>
VkObjectType get_object_type<VkImage>() {
    return VK_OBJECT_TYPE_IMAGE;
}
template<>
VkObjectType get_object_type<VkEvent>() {
    return VK_OBJECT_TYPE_EVENT;
}
template<>
VkObjectType get_object_type<VkQueryPool>() {
    return VK_OBJECT_TYPE_QUERY_POOL;
}
template<>
VkObjectType get_object_type<VkBufferView>() {
    return VK_OBJECT_TYPE_BUFFER_VIEW;
}
template<>
VkObjectType get_object_type<VkImageView>() {
    return VK_OBJECT_TYPE_IMAGE_VIEW;
}
template<>
VkObjectType get_object_type<VkShaderModule>() {
    return VK_OBJECT_TYPE_SHADER_MODULE;
}
template<>
VkObjectType get_object_type<VkPipelineCache>() {
    return VK_OBJECT_TYPE_PIPELINE_CACHE;
}
template<>
VkObjectType get_object_type<VkPipelineLayout>() {
    return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
}
template<>
VkObjectType get_object_type<VkRenderPass>() {
    return VK_OBJECT_TYPE_RENDER_PASS;
}
template<>
VkObjectType get_object_type<VkPipeline>() {
    return VK_OBJECT_TYPE_PIPELINE;
}
template<>
VkObjectType get_object_type<VkDescriptorSetLayout>() {
    return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
}
template<>
VkObjectType get_object_type<VkSampler>() {
    return VK_OBJECT_TYPE_SAMPLER;
}
template<>
VkObjectType get_object_type<VkDescriptorPool>() {
    return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
}
template<>
VkObjectType get_object_type<VkDescriptorSet>() {
    return VK_OBJECT_TYPE_DESCRIPTOR_SET;
}
template<>
VkObjectType get_object_type<VkFramebuffer>() {
    return VK_OBJECT_TYPE_FRAMEBUFFER;
}
template<>
VkObjectType get_object_type<VkCommandPool>() {
    return VK_OBJECT_TYPE_COMMAND_POOL;
}
template<>
VkObjectType get_object_type<VkSurfaceKHR>() {
    return VK_OBJECT_TYPE_SURFACE_KHR;
}
template<>
VkObjectType get_object_type<VkSwapchainKHR>() {
    return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
}
template<>
VkObjectType get_object_type<VkDebugReportCallbackEXT>() {
    return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
}
template<>
VkObjectType get_object_type<VkDisplayKHR>() {
    return VK_OBJECT_TYPE_DISPLAY_KHR;
}
template<>
VkObjectType get_object_type<VkDisplayModeKHR>() {
    return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
}
template<>
VkObjectType get_object_type<VkValidationCacheEXT>() {
    return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
}
template<>
VkObjectType get_object_type<VkAccelerationStructureKHR>() {
    return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
}
template<>
VkObjectType get_object_type<VkDeferredOperationKHR>() {
    return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR;
}
template<typename VKObject>
VkDebugReportObjectTypeEXT get_debug_report_object_type() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkInstance>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkPhysicalDevice>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDevice>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkQueue>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkSemaphore>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkCommandBuffer>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkFence>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDeviceMemory>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkBuffer>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkImage>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkEvent>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkQueryPool>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkBufferView>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkImageView>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkShaderModule>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkPipelineCache>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkPipelineLayout>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkRenderPass>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkPipeline>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDescriptorSetLayout>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkSampler>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDescriptorPool>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDescriptorSet>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkFramebuffer>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkCommandPool>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkSurfaceKHR>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkSwapchainKHR>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDebugReportCallbackEXT>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDisplayKHR>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkDisplayModeKHR>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkValidationCacheEXT>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT;
}
template<>
VkDebugReportObjectTypeEXT get_debug_report_object_type<VkAccelerationStructureKHR>() {
    return VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT;
}

template<typename HandleClass>
vulkan::VulkanObject<HandleClass>::operator HandleClass() const noexcept {
    return m_handle;
}
template<typename HandleClass>
HandleClass vulkan::VulkanObject<HandleClass>::get_handle() const noexcept {
    return m_handle;
}

static std::unordered_map<uint64_t, std::string> s_labels;

template<typename HandleClass>
void vulkan::VulkanObject<HandleClass>::set_debug_label(const char* name) const noexcept {
    if constexpr (debugMarkers) {
        if (r_device.get_supported_extensions().debug_utils) {
            VkDebugUtilsObjectNameInfoEXT label{
                .sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT},
                .pNext {nullptr},
                .objectType {get_object_type<HandleClass>()},
                .objectHandle {reinterpret_cast<uint64_t>(m_handle)},
                .pObjectName {name},
            };
            vkSetDebugUtilsObjectNameEXT(r_device, &label);
        }
        else if (r_device.get_supported_extensions().debug_marker) {
            VkDebugMarkerObjectNameInfoEXT label{
                .sType {VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT},
                .pNext {nullptr},
                .objectType {get_debug_report_object_type<HandleClass>()},
                .object {reinterpret_cast<uint64_t>(m_handle)},
                .pObjectName {name},
            };
            vkDebugMarkerSetObjectNameEXT(r_device, &label);
        }
        s_labels[reinterpret_cast<uint64_t>(m_handle)] = name;
    }
}
template<typename HandleClass>
const std::string& vulkan::VulkanObject<HandleClass>::get_debug_label(uint64_t id) {
    return s_labels[id];
}

template<typename HandleClass>
constexpr vulkan::VulkanObject<HandleClass>::VulkanObject(LogicalDevice& device) noexcept 
    : r_device(device),
    m_handle(VK_NULL_HANDLE) 
{
};
template<typename HandleClass>
constexpr vulkan::VulkanObject<HandleClass>::VulkanObject(LogicalDevice& device, const HandleClass& handle) noexcept 
    : r_device(device),
    m_handle(handle) 
{
};

template class vulkan::VulkanObject<VkInstance>;
template class vulkan::VulkanObject<VkPhysicalDevice>;
template class vulkan::VulkanObject<VkDevice>;
template class vulkan::VulkanObject<VkQueue>;
template class vulkan::VulkanObject<VkSemaphore>;
template class vulkan::VulkanObject<VkCommandBuffer>;
template class vulkan::VulkanObject<VkFence>;
template class vulkan::VulkanObject<VkDeviceMemory>;
template class vulkan::VulkanObject<VkBuffer>;
template class vulkan::VulkanObject<VkImage>;
template class vulkan::VulkanObject<VkEvent>;
template class vulkan::VulkanObject<VkQueryPool>;
template class vulkan::VulkanObject<VkBufferView>;
template class vulkan::VulkanObject<VkImageView>;
template class vulkan::VulkanObject<VkShaderModule>;
template class vulkan::VulkanObject<VkPipelineCache>;
template class vulkan::VulkanObject<VkPipelineLayout>;
template class vulkan::VulkanObject<VkRenderPass>;
template class vulkan::VulkanObject<VkPipeline>;
template class vulkan::VulkanObject<VkDescriptorSetLayout>;
template class vulkan::VulkanObject<VkSampler>;
template class vulkan::VulkanObject<VkDescriptorPool>;
template class vulkan::VulkanObject<VkDescriptorSet>;
template class vulkan::VulkanObject<VkFramebuffer>;
template class vulkan::VulkanObject<VkCommandPool>;
template class vulkan::VulkanObject<VkSurfaceKHR>;
template class vulkan::VulkanObject<VkDisplayKHR>;
template class vulkan::VulkanObject<VkDisplayModeKHR>;
template class vulkan::VulkanObject<VkAccelerationStructureKHR>;