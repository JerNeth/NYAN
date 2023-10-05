#ifndef VKDEBUGLABELS_H
#define VKDEBUGLABELS_H
#pragma once

#include <string>
#include <cassert>

#include "VulkanWrapper/VulkanIncludes.h"

namespace vulkan {

    class LogicalDevice;
    template<typename HandleClass>
    class VulkanObject {
    public:
        operator HandleClass() const noexcept 
        {
            return m_handle;
        }
        HandleClass get_handle() const noexcept 
        {
            return m_handle;
        }
        void set_debug_label(const char* name) const noexcept;
        //static const std::string& get_debug_label(uint64_t id);
    protected:
        explicit constexpr VulkanObject(LogicalDevice& device) noexcept
            : r_device(device),
            m_handle(VK_NULL_HANDLE)
        {
	        
        }
        constexpr VulkanObject(LogicalDevice& device, const HandleClass& handle) noexcept : 
            r_device(device),
            m_handle(handle)  
        {
	        
        }
        constexpr VulkanObject(VulkanObject&& other) noexcept :
			r_device(other.r_device),
			m_handle(other.m_handle)
        {
            other.m_handle = VK_NULL_HANDLE;
        }
        constexpr VulkanObject& operator=(VulkanObject&& other) noexcept
        {
	        if(this != &other)
	        {
                assert(&r_device == &other.r_device);
                std::swap(m_handle, other.m_handle);
               // m_handle = other.m_handle;
               // other.m_handle = VK_NULL_HANDLE;
	        }
            return *this;
        }
        LogicalDevice& r_device;
        HandleClass m_handle { VK_NULL_HANDLE };
    private:
    };
}

#endif