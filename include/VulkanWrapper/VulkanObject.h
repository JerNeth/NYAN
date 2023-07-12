#ifndef VKDEBUGLABELS_H
#define VKDEBUGLABELS_H
#pragma once

#include <string>

namespace vulkan {

    class LogicalDevice;
    template<typename HandleClass>
    class VulkanObject {
    public:
        operator HandleClass() const noexcept;
        HandleClass get_handle() const noexcept;
        void set_debug_label(const char* name) const noexcept;
        //static const std::string& get_debug_label(uint64_t id);
    protected:
        explicit constexpr VulkanObject(LogicalDevice& device) noexcept
            : r_device(device),
            m_handle(VK_NULL_HANDLE)
        {
	        
        }
        constexpr VulkanObject(LogicalDevice& device, const HandleClass& handle) noexcept : r_device(device),
            m_handle(handle)  
        {
	        
        }
        LogicalDevice& r_device;
        HandleClass m_handle;
    private:
    };
}

#endif