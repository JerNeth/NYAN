#ifndef VKWRAPPER_H
#define VKWRAPPER_H
#pragma once
#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <optional>
#include <bitset>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <array>
#include <string>

#include <LogicalDevice.h>
#include <Instance.h>
namespace vk {
	
}

#endif // VKWRAPPER_H!
