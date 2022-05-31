#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"
#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "extracts.glsl"
#include "common.glsl"
#include "lighting.glsl"


layout(std430, push_constant) uniform PushConstants
{
    uint imageBinding;
    uint accBinding;
    uint sceneBinding;
    uint meshBinding;
	vec4 col;
} constants;

layout(location = 0) rayPayloadInEXT hitPayload hitValue;
hitAttributeEXT vec3 attribs;


void main()
{
	Mesh mesh = meshData[constants.meshBinding].meshes[gl_InstanceCustomIndexEXT];
	Material material = materials[nonuniformEXT(mesh.materialBinding)].materials[nonuniformEXT(mesh.materialId)];
    Indices indices = Indices(mesh.indicesAddress);
    Positions positions = Positions(mesh.positionsAddress);
    Uvs uvs = Uvs(mesh.uvsAddress);
    Normals normals = Normals(mesh.normalsAddress);
    Tangents tangents = Tangents(mesh.tangentsAddress);

    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);


    ivec3 ind = indices.i[gl_PrimitiveID];
    vec3 pos = positions.p[ind.x] * barycentrics.x + positions.p[ind.y] * barycentrics.y + positions.p[ind.z] * barycentrics.z;
    vec3 worldPos = gl_WorldToObjectEXT * vec4(pos, 1.0);
    vec3 tangent =  vec3(tangents.t[ind.z]* gl_WorldToObjectEXT);
    vec3 normalS = vec3(normals.n[ind.z]* gl_WorldToObjectEXT);
    vec3 bitangent = cross(normalS, tangent);
    vec2 uv = uvs.u[ind.x] * barycentrics.x + uvs.u[ind.y] * barycentrics.y + uvs.u[ind.z] * barycentrics.z;;
    
	mat3 tangentFrame = mat3(tangent, bitangent, normalS);
    
    Scene scene = scenes[constants.sceneBinding].scene;
	DirectionalLight light1 = scene.dirLight1;
    vec3 viewPos = vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
    
    vec4 pbr = vec4(material.metalness, material.roughness, 0, 0);
    vec4 albedo = textureLod(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), uv, 0);
    vec3 normal = vec3(textureLod(sampler2D(textures2D[nonuniformEXT(material.normalTexId)], samplers[nonuniformEXT(material.normalSampler)]), uv, 0).rg, 0);
    normal.z = 1-normal.x*normal.x - normal.y*normal.y;
    normal = tangentFrame * normal;
    
    vec3 diffuse;
    vec3 specular;
    calcDirLight(albedo.xyz, pbr.xyz, -gl_WorldRayDirectionEXT, normal, light1, specular, diffuse);
	
	hitValue.hitValue = diffuse + specular;
}
