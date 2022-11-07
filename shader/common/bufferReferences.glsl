#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_gpu_shader_int64 : require

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
	vec4 t[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) buffer UvsHalf {
	uint u[];
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer NormalsHalf {
	uint n[];
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer TangentsHalf {
	uint t[]; 
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer TraceRaysIndirect {
	uint width; 
	uint height;
	uint depth;
};

ivec3 get_indices(uint64_t indicesAddress, uint idx) {
	Indices indices = Indices(indicesAddress);
	return indices.i[idx];
}

vec3 get_position(uint64_t positionAddress, uint idx) {
	Positions positions = Positions(positionAddress);
	return positions.p[idx];
}

vec3 get_normal(uint64_t normalsAddress, uint idx) {
	#define NORMALSHALF
	#ifdef NORMALSHALF
	NormalsHalf normalsHalf = NormalsHalf(normalsAddress);
	uint low = normalsHalf.n[idx * 3 / 2 ];
	uint high = normalsHalf.n[idx * 3 / 2  + 1];
	vec2 lowVec = unpackHalf2x16(low);
	vec2 highVec = unpackHalf2x16(high);
	if((idx % 2) == 1)
		return vec3(lowVec.y, highVec.xy);
	else
		return vec3(lowVec.xy, highVec.x);
	#else
	Normals normals = Normals(normalsAddress);
	return normals.n[idx];
	#endif
	#undef NORMALSHALF
}

vec4 get_tangent(uint64_t tangentsAddress, uint idx) {
	#define TANGENTSSHALF
	#ifdef TANGENTSSHALF
	TangentsHalf tangentsHalf = TangentsHalf(tangentsAddress);
	uint low = tangentsHalf.t[idx * 2 ];
	uint high = tangentsHalf.t[idx * 2  + 1];
	vec2 lowVec = unpackHalf2x16(low);
	vec2 highVec = unpackHalf2x16(high);
	return vec4(lowVec, highVec);
	#else
	Tangents tangents = Tangents(tangentsAddress);
	return tangents.t[idx];
	#endif
	#undef TANGENTSSHALF
}

vec2 get_uv(uint64_t uvsAddress, uint idx) {
	#define UVSHALF
	#ifdef UVSHALF
	UvsHalf uvHalf = UvsHalf(uvsAddress);
	uint low = uvHalf.u[idx];
	return unpackHalf2x16(low).xy;
	#else
	Uvs uvs = Uvs(uvsAddress);
	return uvs.u[idx];
	#endif
	#undef UVSHALF
}