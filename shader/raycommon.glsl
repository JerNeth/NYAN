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