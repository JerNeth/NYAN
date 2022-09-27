struct hitPayload
{
	vec3 hitValue;
};

//Taken from Ray Tracing Gems II
mat2 compute_tex_coord_derivatives(mat2 dstDx1x2, vec2 uv0, vec2 uv1, vec2 uv2)
{
	mat2 dtcDuv = mat2(uv1.x - uv0.x,uv1.y - uv0.y ,
						uv2.x - uv0.x, uv2.y - uv0.y);
	return dtcDuv * dstDx1x2;
}

//Taken from Ray Tracing Gems II
void barycentric_world_derivatives(vec3 a1, vec3 a2, out vec3 duDx, out vec3 dvDx) {
	vec3 Nt = cross(a1, a2);
	Nt /= dot(Nt, Nt);
	duDx = cross(a2, Nt);
	dvDx = cross(Nt, a1);
}

//Taken from Ray Tracing Gems II
mat3 world_screen_derivatives(mat4 worldMatrix, mat4 invWorldMatrix, vec4 x) 
{
	//float wMx = dot(vec4(worldMatrix[0][3], worldMatrix[1][3], worldMatrix[2][3], worldMatrix[3][3]), x);
	mat3 dxDxt = transpose(mat3(invWorldMatrix)); //Use transpose to access actual rows instead of columns
	vec3 row3 = vec3(invWorldMatrix[0][3], invWorldMatrix[1][3], invWorldMatrix[2][3]);
	dxDxt[0] -= x.x * row3;
	dxDxt[1] -= x.y * row3;
	dxDxt[2] -= x.z * row3;
	return dxDxt; //transpose
}

//Taken from Ray Tracing Gems II
vec2 depth_gradient(vec4 x, vec3 n, mat4 invWorldMatrix) {
	vec4 n4 = vec4(n, -dot(n, x.xyz));
	n4 = n4 * invWorldMatrix; //Not sure about this
	n4.z = max(abs(n4.z), 0.0001) * sign(n4.z);
	return n4.xy / -n4.z;
}

//Taken from Ray Tracing Gems II
void barycentric_derivatives(vec4 x, vec3 n, vec3 x0, vec3 x1, vec3 x2, mat4 worldMatrix, mat4 invWorldMatrix, out vec2 duDXY, out vec2 dvDXY)
{
	vec3 duDx, dvDx;
	barycentric_world_derivatives(x1- x0, x2 - x0, duDx, dvDx);

	mat3 dxDxt = world_screen_derivatives(worldMatrix, invWorldMatrix, x);

	vec3 duDxt = duDx.x * dxDxt[0] + duDx.y * dxDxt[1] + duDx.z * dxDxt[2];
	vec3 dvDxt = dvDx.x * dxDxt[0] + dvDx.y * dxDxt[1] + dvDx.z * dxDxt[2];

	vec2 ddepthdXY = depth_gradient(x, n, invWorldMatrix);
	float wMx = dot(vec4(worldMatrix[0][3], worldMatrix[1][3], worldMatrix[2][3], worldMatrix[3][3]), x);
	duDXY = (duDxt.xy + duDxt.z * ddepthdXY) * wMx;
	dvDXY = (dvDxt.xy + dvDxt.z * ddepthdXY) * wMx;
}

const uint HitDDGIMiss = 0xFDu;

struct Payload {
	vec3 albedo;
	float opacity;
	vec3 worldPos;
	float metallic;
	vec3 normal;
	float roughness;
	vec3 shadingNormal;
	float hitT;
	uint hitkind;
};
struct PackedPayload {
	vec3 worldPos;
	float hitT;
	uvec4 packed0;
	uvec4 packed1;
};

Payload unpack_payload(PackedPayload packed) {
	Payload payload;
	payload.hitT = packed.hitT;
	payload.worldPos = packed.worldPos;

	payload.albedo.xy = unpackHalf2x16(packed.packed0.x);
	{
		const vec2 tmp = unpackHalf2x16(packed.packed0.y);
		payload.albedo.z = tmp.x;
		payload.opacity = tmp.y;
	}
	payload.normal.xy = unpackHalf2x16(packed.packed0.z);
	{	
		const vec2 tmp = unpackHalf2x16(packed.packed0.w);
		payload.normal.z = tmp.x;
		payload.metallic = tmp.y;
	}
	payload.shadingNormal.xy = unpackHalf2x16(packed.packed1.x);
	{
		const vec2 tmp = unpackHalf2x16(packed.packed1.y);
		payload.shadingNormal.z = tmp.x;
		payload.roughness = tmp.y;
	}
	payload.hitkind = packed.packed1.w;
	return payload;
}

PackedPayload pack_payload(Payload payload) {
	PackedPayload packed;
	packed.hitT = payload.hitT;
	packed.worldPos = payload.worldPos;
	packed.packed0.x = packHalf2x16(payload.albedo.xy);
	packed.packed0.y = packHalf2x16(vec2(payload.albedo.z, payload.opacity));
	packed.packed0.z = packHalf2x16(payload.normal.xy);
	packed.packed0.w = packHalf2x16(vec2(payload.normal.z, payload.metallic));
	packed.packed1.x = packHalf2x16(payload.shadingNormal.xy);
	packed.packed1.y = packHalf2x16(vec2(payload.shadingNormal.z, payload.roughness));
	packed.packed1.z = payload.hitkind;
	packed.packed1.w = payload.hitkind;

	return packed;
}