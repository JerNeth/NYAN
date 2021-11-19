#ifndef GLFWINCLUDES_H
#define GLFWINCLUDES_H
#pragma once

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdocumentation"
	#pragma clang diagnostic ignored "-Wreserved-id-macro"
	#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif
#include <GLFW/glfw3.h>

#ifdef __linux__
#undef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_X11
#undef GLFW_EXPOSE_NATIVE_WAYLAND
#undef GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif _WIN32 
#undef GLFW_EXPOSE_NATIVE_COCOA
#undef GLFW_EXPOSE_NATIVE_X11
#undef GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#ifdef __clang__
	#pragma clang diagnostic pop
#endif
#endif