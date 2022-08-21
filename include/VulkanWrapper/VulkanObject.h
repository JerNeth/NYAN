#ifndef VKDEBUGLABELS_H
#define VKDEBUGLABELS_H
#pragma once



namespace vulkan {
    class LogicalDevice;
    template<typename HandleClass>
    class VulkanObject {
    public:
        operator HandleClass() const noexcept {
            return m_handle;
        }
        HandleClass get_handle() const noexcept {
            return m_handle;
        }
    protected:
        constexpr VulkanObject(LogicalDevice& device) noexcept : r_device(device), m_handle(VK_NULL_HANDLE) {};
        constexpr VulkanObject(LogicalDevice& device, const HandleClass& handle) noexcept : r_device(device), m_handle(handle) {};
        HandleClass m_handle;
        LogicalDevice& r_device;
    private:
    };
}

#endif