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
#include <GLFW/glfw3native.h>
#ifdef __clang__
	#pragma clang diagnostic pop
#endif
#endif