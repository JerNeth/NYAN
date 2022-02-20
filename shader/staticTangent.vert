#version 450

layout(set = 0, binding = 0) uniform SceneMatrices {
    mat4 view;
    mat4 proj;
} sceneMatrices;

layout(std430, push_constant) uniform PushConstants
{
    mat4 model;
} constants;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec4 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat3 fragTangentFrame;

void main() {
    gl_Position = sceneMatrices.proj * sceneMatrices.view * constants.model * vec4(inPosition, 1.0);
    vec3 tangent = vec3(constants.model * vec4(inTangent.xyz, 0));
    vec3 normal = vec3(constants.model * vec4(inNormal.xyz, 0));
    vec3 bitangent = cross(normal, tangent);
    fragTangentFrame = mat3(tangent, bitangent, normal);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}

