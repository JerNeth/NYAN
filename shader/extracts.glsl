#ifndef EXTRACTS_GLSL
#define EXTRACTS_GLSL
#extension GL_EXT_nonuniform_qualifier : require
#include "common.glsl"
#include "bindlessLayouts.glsl"

mat4x3 fetchRowMatrix(vec4 row1, vec4 row2, vec4 row3) {
	return mat4x3(row1.x, row2.x, row3.x,
						row1.y, row2.y, row3.y,
						row1.z, row2.z, row3.z,
						row1.w, row2.w, row3.w);
}
mat4x3 fetchTransformMatrix(Instance instance) {
    return fetchRowMatrix(instance.modelRow1, 
                       instance.modelRow2,
                        instance.modelRow3);
}

struct VertexData
{
    vec3 tangent;
    vec3 normal;
    vec3 bitangent;
    vec3 worldPos;
    vec2 uv;
};

VertexData get_vertex_data(in Mesh mesh, in vec2 barycentricCoords, in uint primitiveId, in mat4x3 objectToWorld) 
{
    VertexData vertexData;
    Indices indices = Indices(mesh.indicesAddress);
    Positions positions = Positions(mesh.positionsAddress);

    const vec3 barycentrics = vec3(1.0 - barycentricCoords.x - barycentricCoords.y, barycentricCoords.x, barycentricCoords.y);
    

    ivec3 ind = get_indices(mesh.indicesAddress, primitiveId);

    vec3 pos = get_position(mesh.positionsAddress, ind.x) * barycentrics.x 
        + get_position(mesh.positionsAddress, ind.y) * barycentrics.y 
        + get_position(mesh.positionsAddress, ind.z) * barycentrics.z;

    vertexData.tangent = normalize(mat3(objectToWorld) * 
        (get_tangent(mesh.tangentsAddress, ind.x) * barycentrics.x 
        + get_tangent(mesh.tangentsAddress, ind.y) * barycentrics.y 
        + get_tangent(mesh.tangentsAddress, ind.z) * barycentrics.z));

    vertexData.normal = normalize(mat3(objectToWorld) * 
        (get_normal(mesh.normalsAddress, ind.x) * barycentrics.x
        + get_normal(mesh.normalsAddress, ind.y) * barycentrics.y 
        + get_normal(mesh.normalsAddress, ind.z) * barycentrics.z));

    vertexData.bitangent = cross(vertexData.normal,vertexData.tangent);

    vertexData.worldPos = objectToWorld   * vec4(pos, 1.0);
    
    vertexData.uv = get_uv(mesh.uvsAddress, ind.x) * barycentrics.x 
        + get_uv(mesh.uvsAddress, ind.y) * barycentrics.y 
        + get_uv(mesh.uvsAddress, ind.z) * barycentrics.z;

    return vertexData;
}

VertexData get_uv_data(in Mesh mesh, in vec2 barycentricCoords, in uint primitiveId, in mat4x3 objectToWorld) 
{
    VertexData vertexData;
    Indices indices = Indices(mesh.indicesAddress);
    Positions positions = Positions(mesh.positionsAddress);

    const vec3 barycentrics = vec3(1.0 - barycentricCoords.x - barycentricCoords.y, barycentricCoords.x, barycentricCoords.y);
    

    ivec3 ind = get_indices(mesh.indicesAddress, primitiveId);
    
    vertexData.uv = get_uv(mesh.uvsAddress, ind.x) * barycentrics.x 
        + get_uv(mesh.uvsAddress, ind.y) * barycentrics.y 
        + get_uv(mesh.uvsAddress, ind.z) * barycentrics.z;

    return vertexData;
}

struct MaterialData
{
    float metalness;
    float roughness;
    vec3 albedo;
    float opacity;
    vec3 shadingNormal;

};

MaterialData get_material_data(in Material material, in VertexData vertexData) 
{
    MaterialData materialData;

    
    vec4 albedo = textureLod(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), vertexData.uv, 0);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    materialData.albedo = albedo.xyz;
    materialData.opacity = albedo.w;
    
    vec4 pbrData = textureLod(sampler2D(textures2D[nonuniformEXT(material.pbrTexId)], samplers[nonuniformEXT(material.pbrSampler)]), vertexData.uv, 0);
    pbrData *= vec4(material.metalness, material.roughness, material.anisotropy, 0);
    materialData.metalness = pbrData.x;
    materialData.roughness = pbrData.y;

    vec2 normalSample = textureLod(sampler2D(textures2D[nonuniformEXT(material.normalTexId)], samplers[nonuniformEXT(material.normalSampler)]), vertexData.uv, 0).rg;
    
    materialData.shadingNormal = tangentSpaceNormal(normalSample, vertexData.normal, vertexData.bitangent, vertexData.tangent);

    return materialData;
}

MaterialData get_albedo_data(in Material material, in VertexData vertexData) 
{
    MaterialData materialData;

    
    vec4 albedo = textureLod(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), vertexData.uv, 0);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    materialData.albedo = albedo.xyz;
    materialData.opacity = albedo.w;
    
    return materialData;
}

vec3 get_viewer_pos(in Scene scene) {
    return vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
}

#endif