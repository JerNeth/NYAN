#version 450
#extension GL_GOOGLE_include_directive : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint meshBinding;
    uint transformBinding;
    uint transformId;
    uint sceneBinding;
} constants;


layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat vec3 fragTangent;
layout(location = 2) in flat vec3 fragNormal;
layout(location = 3) in flat vec3 fragBitangent;

layout(location = 0) out vec4 outColor;

void main() {
	Mesh mesh = meshData[constants.meshBinding].meshes[transforms[constants.transformBinding].transforms[constants.transformId].pad.x];
	DirectionalLight light1 = scenes[constants.sceneBinding].scene.dirLight1;
	DirectionalLight light2 = scenes[constants.sceneBinding].scene.dirLight2;
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    vec4 diffuse = texture(sampler2D(textures2D[material.diffuseTexId], samplers[2]), fragTexCoord);
    vec3 normal = vec3(texture(sampler2D(textures2D[material.normalTexId], samplers[2]), fragTexCoord).rg, 0);
    normal.z = 1-normal.x*normal.x - normal.y*normal.y;
	mat3 tangentFrame = mat3(fragTangent, fragBitangent, fragNormal);
    normal = tangentFrame * normal;
	diffuse.xyz *= dot(normal, light1.dir);
    outColor = diffuse;
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

