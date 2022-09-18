#version 450
#extension GL_GOOGLE_include_directive : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "extracts.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint meshBinding;
    uint instanceBinding;
    uint instanceId;
    uint sceneBinding;
} constants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragTangent;
layout(location = 4) out vec3 fragWorldPos;

void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
    Scene scene = scenes[constants.sceneBinding].scene;
	mat4x3 model = fetchTransformMatrix(instance);
    //Cancel out translations before other transformations
    mat3 modelS = mat3(model);
    //model[3] -=  get_viewer_pos(scene);
	//gl_Position = scene.proj * vec4(mat3(scene.view) * fragWorldPos , 1.0);
    fragWorldPos = model * vec4( inPosition, 1.0);
	gl_Position = scene.proj * scene.view * vec4(fragWorldPos, 1.0);
    vec3 tangent =  inTangent.xyz * modelS;
    vec3 normal = modelS *  inNormal.xyz;
    //tangent = normalize(tangent - dot(tangent, normal) * normal);
//    Uvs uvs = Uvs(mesh.uvs);
   // Normals normals = Normals(mesh.normalsAddress);
    //NormalsHalf normals = NormalsHalf(mesh.normalsAddress);
//    Tangents tangents = Tangents(mesh.tangents);
//    vec3 tangent = vec3(model * vec4(tangents.t[gl_VertexIndex].xyz, 0));
    //vec3 normal = normalize(modelS * normals.n[gl_VertexIndex].xyz);
    //vec3 normal = normalize(modelS *getNormal(mesh.normalsAddress, gl_VertexIndex));
    //vec3 bitangent = cross(normal.xyz, tangent.xyz) * inTangent.w;
    //980 ti
    //46 Entities 57 fps    17.5ms (fetch)
    //46 Entities 57 fps    17.5 ms (attribs)
    //46 Entities 57 fps    17.5 ms (bind)
    //3080
    //67 Entities 17 ms (32 bits)
    //67 Entities 18 ms (16 bits)
    //1.1 / 900
    //fragTexCoord = get_uv(mesh.uvsAddress, gl_VertexIndex);

    fragTexCoord = inTexCoord;
    fragTangent = vec4(tangent, inTangent.w);
    //fragTangent = inTangent;
    fragNormal = normal;
    //fragBitangent = bitangent;
}

