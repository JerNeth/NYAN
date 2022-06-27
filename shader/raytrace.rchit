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
    Scene scene = scenes[constants.sceneBinding].scene;
	Mesh mesh = meshData[constants.meshBinding].meshes[gl_InstanceCustomIndexEXT];
	Material material = materials[nonuniformEXT(mesh.materialBinding)].materials[nonuniformEXT(mesh.materialId)];
    Indices indices = Indices(mesh.indicesAddress);
    Positions positions = Positions(mesh.positionsAddress);

    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);


    ivec3 ind = get_indices(mesh.indicesAddress, gl_PrimitiveID);

    vec3 pos = get_position(mesh.positionsAddress, ind.x) * barycentrics.x 
        + get_position(mesh.positionsAddress, ind.y) * barycentrics.y 
        + get_position(mesh.positionsAddress, ind.z) * barycentrics.z;

    vec3 tangentVertex = normalize(mat3(gl_ObjectToWorldEXT) * 
        (get_tangent(mesh.tangentsAddress, ind.x) * barycentrics.x 
        + get_tangent(mesh.tangentsAddress, ind.y) * barycentrics.y 
        + get_tangent(mesh.tangentsAddress, ind.z) * barycentrics.z));

    vec3 normalVertex = normalize(mat3(gl_ObjectToWorldEXT) * 
        (get_normal(mesh.normalsAddress, ind.x) * barycentrics.x
        + get_normal(mesh.normalsAddress, ind.y) * barycentrics.y 
        + get_normal(mesh.normalsAddress, ind.z) * barycentrics.z));

    vec3 bitangentVertex = cross(normalVertex, tangentVertex);

    vec3 worldPos = gl_ObjectToWorldEXT   * vec4(pos, 1.0);
    vec2 duDXY;
    vec2 dvDXY;
    
//    mat4 tmp = mat4( gl_WorldToObjectEXT);
//    tmp[3][3] = 1;
//    mat4 invWorldMatrix = tmp * scene.invViewProj;
//    tmp = mat4(gl_ObjectToWorldEXT);
//    tmp[3][3] = 1;
//    mat4 worldMatrix = scene.viewProj * tmp;
    //void barycentric_derivatives(vec4 x, vec3 n, vec3 x0, vec3 x1, vec3 x2, mat4 worldMatrix, mat4 invWorldMatrix, out vec2 duDXY, out vec2 dvDXY)
    //barycentric_derivatives(vec4(worldPos, 1.0), normalS, positions.p[ind.x], positions.p[ind.y], positions.p[ind.z], worldMatrix, invWorldMatrix, duDXY, dvDXY);

    vec2 uv = get_uv(mesh.uvsAddress, ind.x) * barycentrics.x 
        + get_uv(mesh.uvsAddress, ind.y) * barycentrics.y 
        + get_uv(mesh.uvsAddress, ind.z) * barycentrics.z;
    
    
	DirectionalLight light1 = scene.dirLight1;
    vec3 viewPos = vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
    
    float metalness = material.metalness;
    float roughness = material.roughness;
    vec4 albedo = textureLod(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), uv, 0);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    vec2 normalSample = textureLod(sampler2D(textures2D[nonuniformEXT(material.normalTexId)], samplers[nonuniformEXT(material.normalSampler)]), uv, 0).rg;
    vec3 normal = tangentSpaceNormal(normalSample, normalVertex, bitangentVertex, tangentVertex);

    
    vec4 diffuse = vec4(0.0);
    vec4 specular= vec4(0.0);
    float NdotL = max(dot(normal, light1.dir), 0.0);
    if(NdotL > 0) {
            

        specular.a = albedo.a;
        diffuse.a = albedo.a;
        uint  rayFlags = gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT | gl_RayFlagsTerminateOnFirstHitEXT;
        float tMin     = 0.01;
        float tMax     = 10000.0;
        shadowed = 0.0;
        traceRayEXT(accelerationStructures[constants.accBinding], // acceleration structure
                rayFlags,       // rayFlags
                0xFF,           // cullMask
                0,              // sbtRecordOffset
                0,              // sbtRecordStride
                1,              // missIndex
                worldSpacePos.xyz,     // ray origin
                tMin,           // ray min range
                light1.dir,  // ray direction
                tMax,           // ray max range
                1               // payload (location = 0)
        );
        if(shadowed > 0)
            calcDirLight(albedo.xyz, metalness, roughness, viewVec, normal, light1, specular, diffuse);
        specular.rgb *= shadowed;
        diffuse.rgb *= shadowed;
    }
	
	hitValue.hitValue = diffuse.xyz + specular.xyz;
}
