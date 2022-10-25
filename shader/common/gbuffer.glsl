#ifndef GBUFFER_GLSL
#define GBUFFER_GLSL


struct GBufferData {
	vec4 data0; //Albedo.xyz, opacity (?)
	vec4 data1; //Octahedral normal in xyz, roughness in w
	vec4 data2; //metalness in x, emissive in xyz
};

GBufferData encode_gbuffer_data(in MaterialData materialData) {
	GBufferData gBufferData;
    gBufferData.data0 = vec4(materialData.albedo, materialData.opacity);
    gBufferData.data1 = vec4(pack1212(zero_to_one(get_octahedral_coords(materialData.shadingNormal))), materialData.roughness);
    gBufferData.data2 = vec4(materialData.metalness, materialData.emissive.xyz);
	return gBufferData;
}

MaterialData decode_gbuffer_data(in GBufferData gBufferData) {
	MaterialData materialData;
    materialData.shadingNormal = get_octahedral_direction(one_to_minus_one(unpack1212(gBufferData.data1.xyz)));
    materialData.metalness = gBufferData.data2.x;
    materialData.roughness = gBufferData.data1.w;
    materialData.albedo = gBufferData.data0.xyz;
    materialData.opacity = gBufferData.data0.w;
    materialData.emissive = gBufferData.data2.yzw;
	return materialData;
}

#endif