#ifndef VKMAXVALS_H
#define VKMAXVALS_H
#pragma once

namespace vulkan {
	constexpr uint8_t MAX_BINDINGS = 16u;
	constexpr uint8_t MAX_DESCRIPTOR_SETS = 8u;
	constexpr uint8_t MAX_ATTACHMENTS = 8u;
	constexpr uint8_t MAX_SETS_PER_POOL = 16u;
	constexpr uint8_t PUSH_CONSTANT_SIZE = 128u;
	//32 on most 1.2 Device (64 on AMD) see maxVertexInputAttributes
	constexpr uint8_t MAX_VERTEX_ATTRIBUTES = 32u;
	constexpr uint8_t MAX_VERTEX_BINDINGS = 32u;
	constexpr uint8_t MAX_VERTEX_INPUTS = 16u;
}
#endif