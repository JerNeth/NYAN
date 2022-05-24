#version 450


layout(std430, push_constant) uniform PushConstants
{
    uint meshId;
    uint materialBinding;
    uint materialId;
    uint padding;
    layout(column_major) mat4x3 model;
    //layout(column_major) mat4x4 view;
    layout(column_major) mat4x4 view;
    layout(column_major) mat4x4 proj;
} constants;
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

void main() {

	const vec3 positions[24] = vec3[24](vec3(-0.5f, -0.5f, 0.5f ),
										vec3(0.5f, -0.5f, 0.5f  ),
										vec3(0.5f, 0.5f, 0.5f   ),
										vec3(-0.5f, 0.5f, 0.5f  ),
										vec3(0.5f, -0.5f, -0.5f ),
										vec3(0.5f, -0.5f, 0.5f  ),
										vec3(0.5f, 0.5f, 0.5f   ),
										vec3(0.5f, 0.5f, -0.5f  ),
										vec3(-0.5f, 0.5f, -0.5f ),
										vec3( 0.5f, 0.5f, -0.5f ),
										vec3( 0.5f, 0.5f, 0.5f  ),
										vec3(-0.5f, 0.5f, 0.5f  ),
										vec3(-0.5f, -0.5f, -0.5f),
										vec3(-0.5f, -0.5f, 0.5f ),
										vec3(-0.5f, 0.5f, 0.5f  ),
										vec3(-0.5f, 0.5f, -0.5f ),
										vec3(-0.5f, -0.5f, -0.5f),
										vec3( 0.5f, -0.5f, -0.5f),
										vec3( 0.5f, -0.5f, 0.5f ),
										vec3(-0.5f, -0.5f, 0.5f ),
										vec3(-0.5f, -0.5f, -0.5f),
										vec3(0.5f, -0.5f, -0.5f ),
										vec3(0.5f, 0.5f, -0.5f  ),
										vec3(-0.5f, 0.5f, -0.5f )
										);
										
	gl_Position = constants.proj * constants.view * vec4( constants.model *vec4( inPosition, 1.0), 1.0);
    vec3 tangent = vec3(constants.model * vec4(inTangent.xyz, 0));
    vec3 normal = vec3(constants.model * vec4(inNormal.xyz, 0));
    vec3 bitangent = cross(normal, tangent);
    fragTexCoord = inTexCoord;
    fragTangent = tangent;
    fragNormal = normal;
    fragBitangent = bitangent;
}

