#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"
#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"


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

	DirectionalLight light1 = scenes[constants.sceneBinding].scene.dirLight1;
    
    vec4 diffuse = texture(sampler2D(textures2D[nonuniformEXT(material.diffuseTexId)], samplers[2]), uv);
    vec3 normal = vec3(texture(sampler2D(textures2D[material.normalTexId], samplers[2]), uv).rg, 0);
    normal.z = 1-normal.x*normal.x - normal.y*normal.y;
    normal = tangentFrame * normal;
	diffuse.xyz *= dot(normal, light1.dir);
	
	hitValue.hitValue = diffuse.xyz;
}
