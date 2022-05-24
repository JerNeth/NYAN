#version 450


layout(std430, push_constant) uniform PushConstants
{
    uint materialBinding;
    uint materialId;
    uint transformBinding;
    uint transformId;
    uint sceneBinding;
} constants;

struct Transform {
	vec4 modelRow1;
	vec4 modelRow2;
	vec4 modelRow3;
    vec4 pad;
};
layout(set = 0, binding = 0) buffer Transforms  {
	Transform transforms[];
} transforms [4096];
layout(set = 0, binding = 0) buffer Scenes  {
    layout(column_major) mat4x4 view;
    layout(column_major) mat4x4 proj;
} scene [4096];
//layout(set = 0, binding = 0) buffer ssbos [];
//layout(set = 0, binding = 1) uniform ubos[];
//layout(set = 0, binding = 2) uniform sampler samplers[];
//layout(set = 0, binding = 3) uniform texture2D textures[];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragBitangent;

mat4x3 fetchRowMatrix(vec4 row1, vec4 row2, vec4 row3) {
	return mat4x3(row1.x, row2.x, row3.x,
						row1.y, row2.y, row3.y,
						row1.z, row2.z, row3.z,
						row1.w, row2.w, row3.w);
}
mat4x3 fetchTransformMatrix(uint transformBinding, uint transformId) {
    return fetchRowMatrix(transforms[transformBinding].transforms[transformId].modelRow1, 
                        transforms[transformBinding].transforms[transformId].modelRow2,
                        transforms[transformBinding].transforms[transformId].modelRow3);
}

void main() {
//
//	const vec3 positions[24] = vec3[24](vec3(-0.5f, -0.5f, 0.5f ),
//										vec3(0.5f, -0.5f, 0.5f  ),
//										vec3(0.5f, 0.5f, 0.5f   ),
//										vec3(-0.5f, 0.5f, 0.5f  ),
//										vec3(0.5f, -0.5f, -0.5f ),
//										vec3(0.5f, -0.5f, 0.5f  ),
//										vec3(0.5f, 0.5f, 0.5f   ),
//										vec3(0.5f, 0.5f, -0.5f  ),
//										vec3(-0.5f, 0.5f, -0.5f ),
//										vec3( 0.5f, 0.5f, -0.5f ),
//										vec3( 0.5f, 0.5f, 0.5f  ),
//										vec3(-0.5f, 0.5f, 0.5f  ),
//										vec3(-0.5f, -0.5f, -0.5f),
//										vec3(-0.5f, -0.5f, 0.5f ),
//										vec3(-0.5f, 0.5f, 0.5f  ),
//										vec3(-0.5f, 0.5f, -0.5f ),
//										vec3(-0.5f, -0.5f, -0.5f),
//										vec3( 0.5f, -0.5f, -0.5f),
//										vec3( 0.5f, -0.5f, 0.5f ),
//										vec3(-0.5f, -0.5f, 0.5f ),
//										vec3(-0.5f, -0.5f, -0.5f),
//										vec3(0.5f, -0.5f, -0.5f ),
//										vec3(0.5f, 0.5f, -0.5f  ),
//										vec3(-0.5f, 0.5f, -0.5f )
//										);
	mat4x3 model = fetchTransformMatrix(constants.transformBinding,constants.transformId);
	gl_Position = scene[constants.sceneBinding].proj * scene[constants.sceneBinding].view * vec4( model *vec4( inPosition, 1.0), 1.0);
    vec3 tangent = vec3(model * vec4(inTangent.xyz, 0));
    vec3 normal = vec3(model * vec4(inNormal.xyz, 0));
    vec3 bitangent = cross(normal, tangent);
    fragTexCoord = inTexCoord;
    fragTangent = tangent;
    fragNormal = normal;
    fragBitangent = bitangent;
}

