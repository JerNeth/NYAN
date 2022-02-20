cmake_minimum_required (VERSION 3.14)


file(GLOB OPTICK_SRC "src/*.*")
source_group("OptickCore" FILES ${OPTICK_SRC})


option(OPTICK_ENABLED "Enable profiling with Optick" OFF)

if(NOT OPTICK_ENABLED)
	message(STATUS "Optick is disabled")
	# add dummy target as a replacement
	add_library(OptickCore STATIC ${OPTICK_SRC})
	target_include_directories(OptickCore PUBLIC "src")
	target_compile_definitions(OptickCore PUBLIC USE_OPTICK=0)
	return()
endif()

option(OPTICK_USE_VULKAN "Built-in support for Vulkan" ON)


add_library(OptickCore STATIC ${OPTICK_SRC})
target_include_directories(OptickCore
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
)

set_target_properties(OptickCore 
	PROPERTIES 
		PUBLIC_HEADER "${CMAKE_CURRENT_LIST_DIR}/src/optick.h;${CMAKE_CURRENT_LIST_DIR}/src/optick.config.h"
		DEBUG_POSTFIX d # So that we can install debug and release side by side
)

target_compile_definitions(OptickCore PRIVATE OPTICK_EXPORTS=1)

target_link_libraries(OptickCore PUBLIC Vulkan::Vulkan)
target_compile_definitions(OptickCore PRIVATE OPTICK_ENABLE_GPU=1)

target_compile_options(OptickCore PRIVATE -DNOMINMAX)
if(MSVC)
	# temporary solution to unblock C++17 users
	target_compile_definitions(OptickCore PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()
