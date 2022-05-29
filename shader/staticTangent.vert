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
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragBitangent;


void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	mat4x3 model = fetchTransformMatrix(instance);
	gl_Position = scenes[constants.sceneBinding].scene.viewProj * vec4( model *vec4( inPosition, 1.0), 1.0);
    vec3 tangent = normalize(vec3(model * vec4(inTangent.xyz, 0)));
    vec3 normal = normalize(vec3(model * vec4(inNormal.xyz, 0)));
//    Uvs uvs = Uvs(mesh.uvs);
//    Normals normals = Normals(mesh.normals);
//    Tangents tangents = Tangents(mesh.tangents);
//    vec3 tangent = vec3(model * vec4(tangents.t[gl_VertexIndex].xyz, 0));
//    vec3 normal = vec3(model * vec4(normals.n[gl_VertexIndex].xyz, 0));
    vec3 bitangent = cross(normal, tangent);
    //980 ti
    //46 Entities 57 fps    17.5ms (fetch)
    //46 Entities 57 fps    17.5 ms (attribs)
    //46 Entities 57 fps    17.5 ms (bind)
    //3080
    //67 Entities 17 ms (32 bits)
    //67 Entities 18 ms (16 bits)
    //1.1 / 900
    //fragTexCoord = uvs.u[gl_VertexIndex];

    fragTexCoord = inTexCoord;
    fragTangent = tangent;
    fragNormal = normal;
    fragBitangent = bitangent;
}

