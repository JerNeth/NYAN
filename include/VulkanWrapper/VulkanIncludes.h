#ifndef VKVULKANINCLUDES_H
#define VKVULKANINCLUDES_H
#ifdef NDEBUG
constexpr bool debug = false;
#else
#define VMA_RECORDING_ENABLED 1
constexpr bool debug = true;
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

#ifdef _WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
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
#define DEBUGSUBMISSIONS
#include <optick.h>
#include <array>
#include <bitset>
#include <vector>
#include <fstream>
#include <assert.h>
#include <unordered_map>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <iostream>
#include <new>
#include <cstddef>
#include <thread>
#endif   //!VKVULKANINCLUDES_H