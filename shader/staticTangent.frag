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

    
layout(set = 0, binding = 0, scalar) buffer MeshData  {
	Mesh meshes[];
} meshData [8 * 1024];

struct DirectionalLight {
	vec3 dir;
	float intensity;
	vec3 color;
	bool enabled;
};
struct PointLight {
	vec3 pos;
	float intensity;
	vec3 color;
	float attenuationDistance;
};
layout(set = 0, binding = 0) buffer Scenes  {
    layout(column_major) mat4x4 view;
    layout(column_major) mat4x4 proj;
    layout(column_major) mat4x4 invView;
    layout(column_major) mat4x4 invProj;
	DirectionalLight dirLight1;
	DirectionalLight dirLight2;
	uint numLights;
	uint pad;
	uint pad1;
	uint pad2;
	PointLight pointLights[8];
} scenes [8 * 1024];

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
} materials [8 * 1024];

layout(set = 0, binding = 0) buffer Transforms  {
	Transform transforms[];
} transforms [8 * 1024];
//layout(set = 0, binding = 1) uniform ubos[];
layout(set = 0, binding = 2) uniform sampler samplers[256];
layout(set = 0, binding = 3) uniform texture2D textures[512 * 1024];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat vec3 fragTangent;
layout(location = 2) in flat vec3 fragNormal;
layout(location = 3) in flat vec3 fragBitangent;

layout(location = 0) out vec4 outColor;

void main() {
	Mesh mesh = meshData[constants.meshBinding].meshes[transforms[constants.transformBinding].transforms[constants.transformId].pad.x];
	DirectionalLight light1 = scenes[constants.sceneBinding].dirLight1;
	DirectionalLight light2 = scenes[constants.sceneBinding].dirLight2;
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    vec4 diffuse = texture(sampler2D(textures[material.diffuseTexId], samplers[2]), fragTexCoord);
    vec3 normal = vec3(texture(sampler2D(textures[material.normalTexId], samplers[2]), fragTexCoord).rg, 0);
    normal.z = 1-normal.x*normal.x - normal.y*normal.y;
	mat3 tangentFrame = mat3(fragTangent, fragBitangent, fragNormal);
    normal = tangentFrame * normal;
	diffuse.xyz *= dot(normal, light1.dir);
    outColor = diffuse;
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

