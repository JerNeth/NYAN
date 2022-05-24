#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require

layout(buffer_reference, scalar, buffer_reference_align = 8) buffer Indices {
	ivec3 i[];
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer Positions {
	vec3 p[];
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer Uvs {
	vec2 u[];
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer Normals {
	vec3 n[];
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer Tangents {
	vec3 t[];
};
