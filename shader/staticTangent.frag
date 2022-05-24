#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, push_constant) uniform PushConstants
{
    uint meshBinding;
    uint transformBinding;
    uint transformId;
    uint sceneBinding;
} constants;

struct Transform {
	vec4 modelRow1;
	vec4 modelRow2;
	vec4 modelRow3;
    ivec4 pad;
};
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer Indices {
    ivec3 i[];
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

struct Mesh {
	uint materialBinding;
	uint materialId;
	Indices indices;
	Uvs uvs;
	Normals normals;
	Tangents tangents;
};
    

layout(set = 0, binding = 0) buffer MeshData  {
	Mesh meshes[];
} meshData [4096];


//layout(set = 0, binding = 0) buffer SSBO  {
//    
//} ssbos [];
struct Material {
	uint diffuseTexId;
	uint normalTexId;
	uint pad;
	float shininessFactor;
	vec3 ambientColor;
	float ambientFactor;
	vec3 diffuseColor;
	float diffuseFactor;
};
layout(set = 0, binding = 0) buffer Materials  {
	Material materials[];
} materials [4096];

layout(set = 0, binding = 0) buffer Transforms  {
	Transform transforms[];
} transforms [4096];
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
	Mesh mesh = meshData[constants.meshBinding].meshes[transforms[constants.transformBinding].transforms[constants.transformId].pad.x];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    vec4 diffuse = texture(sampler2D(textures[material.diffuseTexId], samplers[2]), fragTexCoord);
    outColor = diffuse;
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

