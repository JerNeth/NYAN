//#define VK_USE_PLATFORM_WIN32_KHR
#ifdef __clang__
#pragma clang diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning( disable : 26812 )
#endif
#include "volk.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#define VK_NO_PROTOTYPES

#define VMA_IMPLEMENTATION
#define NOMINMAX
#ifndef NDEBUG
	#define VMA_RECORDING_ENABLED 1
#endif
#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wnullability-completeness"
	#pragma clang diagnostic ignored "-Wunused-variable"
	#pragma clang diagnostic ignored "-Wdocumentation"
	#pragma clang diagnostic ignored "-Wnullability-extension"
#endif
#include "vk_mem_alloc.h"
#ifdef __clang__
	#pragma clang diagnostic pop
#endif