#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "extracts.glsl"
#include "common.glsl"
#include "lighting.glsl"
#include "raycommon.glsl"


layout(std430, push_constant) uniform PushConstants
{
    uint accBinding;
    uint sceneBinding;
    uint meshBinding;
    uint imageBinding;
	vec4 col;
} constants;
layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[ACC_COUNT];

layout(location = 0) rayPayloadInEXT hitPayload hitValue;
layout(location = 1) rayPayloadInEXT float shadowed;

hitAttributeEXT vec2 baryCoord;


void main()
{
    Scene scene = scenes[constants.sceneBinding].scene;
	Mesh mesh = meshData[nonuniformEXT(constants.meshBinding)].meshes[nonuniformEXT(gl_InstanceCustomIndexEXT)];
	Material material = materials[nonuniformEXT(mesh.materialBinding)].materials[nonuniformEXT(mesh.materialId)];

    const VertexData vertexData = get_vertex_data(mesh, baryCoord, gl_PrimitiveID, gl_ObjectToWorldEXT);
    const MaterialData materialData = get_material_data(material, vertexData);
    
	DirectionalLight light = scene.dirLight;
    vec3 viewPos = get_viewer_pos(scene);
    vec3 viewVec = normalize(viewPos - vertexData.worldPos.xyz);
    
    vec4 diffuse = vec4(0.0);
    vec4 specular= vec4(0.0);
    float NdotL = max(dot(materialData.shadingNormal, light.dir), 0.0);
    if(NdotL > 0) {
            

        specular.a = materialData.opacity;
        diffuse.a = materialData.opacity;
        uint  rayFlags = gl_RayFlagsSkipClosestHitShaderEXT | gl_RayFlagsTerminateOnFirstHitEXT;
        float tMin     = 0.01;
        float tMax     = 10000.0;
        shadowed = 0.0;
        traceRayEXT(accelerationStructures[constants.accBinding], // acceleration structure
                rayFlags,       // rayFlags
                0xFF,           // cullMask
                0,              // sbtRecordOffset
                0,              // sbtRecordStride
                1,              // missIndex
                vertexData.worldPos.xyz,     // ray origin
                tMin,           // ray min range
                light.dir,  // ray direction
                tMax,           // ray max range
                1               // payload (location = 0)
        );
        //if(shadowed > 0)
        //    calcDirLight(materialData.albedo.xyz, materialData.metalness, materialData.roughness, viewVec, materialData.shadingNormal, light, specular, diffuse);
        specular.xyz = materialData.albedo;
        diffuse.xyz = materialData.albedo;
        specular.rgb *= shadowed;
        diffuse.rgb *= shadowed;
    }
	
	hitValue.hitValue = diffuse.xyz + specular.xyz;
}
