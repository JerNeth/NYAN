#pragma once
#ifndef VKVULKANINCLUDES_H
#define VKVULKANINCLUDES_H

#ifndef NOMINMAX
# define INTERNAL_NOMINMAX
# define NOMINMAX
#endif
#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wnullability-completeness"
	#pragma clang diagnostic ignored "-Wunused-variable"
	#pragma clang diagnostic ignored "-Wdocumentation"
	#pragma clang diagnostic ignored "-Wnullability-extension"
#endif

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

#ifdef NDEBUG
constexpr bool debug = false;
constexpr bool debugMarkers = false;
#else
#define VMA_RECORDING_ENABLED 1
#define INTERNAL_VMA_RECORDING_ENABLED
constexpr bool debug = true;
constexpr bool debugMarkers = true;
#endif
#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning( disable : 26812 )
#endif
#ifdef __clang__
#pragma clang diagnostic push
#endif
#include "vk_mem_alloc.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
	#pragma clang diagnostic pop
#endif


#ifndef INTERNAL_NOMINMAX
# undef NOMINMAX
#endif

#ifdef INTERNAL_VMA_RECORDING_ENABLED
#undef VMA_RECORDING_ENABLED
#endif

//#include "glfwIncludes.h"
#endif   //!VKVULKANINCLUDES_H