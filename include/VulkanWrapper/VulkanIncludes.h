#ifndef VKVULKANINCLUDES_H
#define VKVULKANINCLUDES_H
#ifdef NDEBUG
constexpr bool debug = false;
#else
#define VMA_RECORDING_ENABLED 1
constexpr bool debug = true;
#endif
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#define NOMINMAX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop
#undef VMA_RECORDING_ENABLED
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "MaxVals.h"
#include "spirv_cross.hpp"

#endif// VKVULKANINCLUDES_H