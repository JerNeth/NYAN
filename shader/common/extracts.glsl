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


void flip_backfacing_normal(inout VertexData vertexData, in bool backFacing) 
{
    if(backFacing) {
        vertexData.normal = -vertexData.normal;
        vertexData.tangent = -vertexData.tangent;
        vertexData.bitangent = -vertexData.bitangent;
    }
}

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

    vec4 tangent = get_tangent(mesh.tangentsAddress, ind.x) * barycentrics.x 
                + get_tangent(mesh.tangentsAddress, ind.y) * barycentrics.y 
                + get_tangent(mesh.tangentsAddress, ind.z) * barycentrics.z;
    vertexData.tangent = normalize(mat3(objectToWorld) *tangent.xyz);

    vertexData.normal = normalize(mat3(objectToWorld) * 
        (get_normal(mesh.normalsAddress, ind.x) * barycentrics.x
        + get_normal(mesh.normalsAddress, ind.y) * barycentrics.y 
        + get_normal(mesh.normalsAddress, ind.z) * barycentrics.z));

    orthonormalize(vertexData.normal, vertexData.tangent, vertexData.bitangent);
    vertexData.bitangent *= tangent.w;

    vertexData.worldPos = objectToWorld * vec4(pos, 1.0);
    
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
MaterialData get_material_data(in Material material, in VertexData vertexData) 
{
    MaterialData materialData;
    
    vec4 albedo = vec4(1.f);
    if(material.albedoTexId != INVALID_BINDING) 
        albedo = texture(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), vertexData.uv);
        //albedo = textureLod(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), vertexData.uv, 0);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    materialData.albedo = albedo.xyz;
    materialData.opacity = albedo.w;
    
    vec4 pbrData = vec4(0.f, 1.f, 1.f, 0.f);
    if(material.pbrTexId != INVALID_BINDING)
        pbrData = pow(texture(sampler2D(textures2D[nonuniformEXT(material.pbrTexId)], samplers[nonuniformEXT(material.pbrSampler)]), vertexData.uv), vec4(1.f /2.2));
        //pbrData = pow(textureLod(sampler2D(textures2D[nonuniformEXT(material.pbrTexId)], samplers[nonuniformEXT(material.pbrSampler)]), vertexData.uv, 0), vec4(1.f /2.2));
    materialData.roughness = pbrData.g * material.roughness;
    materialData.metalness = pbrData.b * material.metalness;

    materialData.shadingNormal = vertexData.normal;
    vec2 normalSample = vec2(0.5f);
    if(material.normalTexId != INVALID_BINDING) {
        normalSample =  pow(texture(sampler2D(textures2D[nonuniformEXT(material.normalTexId)], samplers[nonuniformEXT(material.normalSampler)]), vertexData.uv).xy, vec2(1.f /2.2f)); 
        //normalSample =  pow(textureLod(sampler2D(textures2D[nonuniformEXT(material.normalTexId)], samplers[nonuniformEXT(material.normalSampler)]), vertexData.uv, 0).xy, vec2(1.f /1.0)); 
        materialData.shadingNormal = tangentSpaceNormal(normalSample, vertexData.normal, vertexData.tangent, vertexData.bitangent);
    }//Temporary fix for wrong sampling of linear textures

    vec3 emissive = vec3(0.f);
    if(material.emissiveTexId != INVALID_BINDING)
        emissive = pow(texture(sampler2D(textures2D[nonuniformEXT(material.emissiveTexId)], samplers[nonuniformEXT(material.emissiveSampler)]), vertexData.uv).rgb, vec3(1.f /2.2));
        //emissive = pow(textureLod(sampler2D(textures2D[nonuniformEXT(material.emissiveTexId)], samplers[nonuniformEXT(material.emissiveSampler)]), vertexData.uv, 0).rgb, vec3(1.f /2.2));
    emissive *= vec3(material.emissive_R, material.emissive_G, material.emissive_B);
    materialData.emissive = emissive;
    return materialData;
}

void computeTangentSpace(inout VertexData vertexData, in vec3 dPdx, in vec3 dPdy, in vec2 dSTdx, in vec2 dSTdy) 
{
 
    vec3 sigmaX = dPdx - dot(dPdx, vertexData.normal) * vertexData.normal;
    vec3 sigmaY = dPdy - dot(dPdy, vertexData.normal) * vertexData.normal;
    float flipSign = sign_not_zero(dot(dPdy, cross(vertexData.normal, dPdx)));


    float det = dot(dSTdx, vec2(dSTdy.y, - dSTdy.x));
    float signDet = sign_not_zero(det);

    vec2 invC0 = signDet * vec2(dSTdy.y, -dSTdx.y);
    vec3 vT = sigmaX * invC0.x + sigmaY * invC0.y;
    if(abs(det) > 0.0) vT = normalize(vT);
    vec3 vB = (signDet * flipSign) * cross(vertexData.normal, vT);

    vertexData.tangent = vT;
    vertexData.bitangent = vB;

}

MaterialData get_albedo_data(in Material material, in VertexData vertexData) 
{
    MaterialData materialData;

    
    vec4 albedo = vec4(1.f);
    if(material.albedoTexId != INVALID_BINDING) 
        albedo = textureLod(sampler2D(textures2D[nonuniformEXT(material.albedoTexId)], samplers[nonuniformEXT(material.albedoSampler)]), vertexData.uv, 0);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    materialData.albedo = albedo.xyz;
    materialData.opacity = albedo.w;
    
    return materialData;
}

vec3 get_viewer_pos(in Scene scene) {
    return vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
}

#endif