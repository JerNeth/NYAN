#pragma once
#ifndef VKDEBUGLABELS_H
#define VKDEBUGLABELS_H
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
namespace vulkan {
    class DebugLabel {
    public:
    protected:
        constexpr DebugLabel() noexcept;
        void set_debug_label(const char* name);
    private:
    }
}

#endif