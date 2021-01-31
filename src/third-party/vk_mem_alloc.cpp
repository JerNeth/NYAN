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