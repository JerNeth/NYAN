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
#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wnullability-completeness"
	#pragma clang diagnostic ignored "-Wunused-variable"
	#pragma clang diagnostic ignored "-Wdocumentation"
	#pragma clang diagnostic ignored "-Wnullability-extension"
#endif
#ifdef _MSC_VER
	#pragma warning(push, 0)
	#pragma warning( disable : 26812 )
#endif
#include "vk_mem_alloc.h"
#ifdef _MSC_VER
	#pragma warning(pop)
#endif
#ifdef __clang__
	#pragma clang diagnostic pop
#endif


#undef VMA_RECORDING_ENABLED
#ifdef __clang__
	#pragma clang diagnostic push
#endif
#ifdef _MSC_VER
	#pragma warning(push, 0)
	#pragma warning( disable : 26812 )
#endif
#include "vulkan\vulkan.h"
#ifdef _MSC_VER
	#pragma warning(pop)
#endif
#ifdef __clang__
	#pragma clang diagnostic pop
#endif
#include "glfwIncludes.h"

#include "MaxVals.h"

#ifdef __clang__
	#pragma clang diagnostic push
#endif
#ifdef _MSC_VER
	#pragma warning(push, 0)
	#pragma warning( disable : 26812 )
#endif
#include "spirv_cross.hpp"
#ifdef _MSC_VER
	#pragma warning(pop)
#endif
#ifdef __clang__
	#pragma clang diagnostic pop
#endif
#endif// VKVULKANINCLUDES_H