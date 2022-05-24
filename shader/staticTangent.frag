#version 450
#extension GL_KHR_vulkan_glsl : enable
layout(std430, push_constant) uniform PushConstants
{
    uint meshId;
    uint materialBinding;
    uint materialId;
    uint padding;
    layout(row_major) mat4x3 model;
} constants;

//layout(set = 0, binding = 0) buffer SSBO  {
//    
//} ssbos [];
struct Material {
	uint diffuseTexId;
	uint normalTexId;
	vec3 ambientColor;
	float ambientFactor;
	vec3 diffuseColor;
	float diffuseFactor;
	float shininessFactor;
};
layout(set = 0, binding = 0) buffer Materials  {
	Material materials[];
} materials [4096];
//layout(set = 0, binding = 1) uniform ubos[];
layout(set = 0, binding = 2) uniform sampler samplers[4096];
layout(set = 0, binding = 3) uniform texture2D textures[4096];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat vec3 fragTangent;
layout(location = 2) in flat vec3 fragNormal;
layout(location = 3) in flat vec3 fragBitangent;

layout(location = 0) out vec4 outColor;

void main() {
	Material material = materials[constants.materialBinding].materials[constants.materialId];
    vec4 diffuse = texture(sampler2D(textures[material.diffuseTexId], samplers[2]), fragTexCoord);
    outColor = diffuse;
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

