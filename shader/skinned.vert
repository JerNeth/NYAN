#version 450
//#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 0) uniform SceneMatrices {
    mat4 view;
    mat4 proj;
} sceneMatrices;
layout(set = 2, binding = 0) uniform BoneMatrices {
    mat4x3 boneTransform[64];
} boneMatrices;
layout(std430, push_constant) uniform PushConstants
{
    mat4 model;
} constants;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uvec4 inBoneIndices;
layout(location = 3) in vec4 inBoneWeights;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    vec3 boneSpace;
    //Have to bench if this is better or worse than no check
    if(inBoneWeights.x != 0)
        boneSpace += (boneMatrices.boneTransform[inBoneIndices.x] * vec4(inPosition, 1.0)) * inBoneWeights.x;
    if(inBoneWeights.y != 0)
        boneSpace += (boneMatrices.boneTransform[inBoneIndices.y] * vec4(inPosition, 1.0))  * inBoneWeights.y;
    if(inBoneWeights.z != 0)
        boneSpace += (boneMatrices.boneTransform[inBoneIndices.z] * vec4(inPosition, 1.0))  * inBoneWeights.z;
    if(inBoneWeights.w != 0)
        boneSpace += (boneMatrices.boneTransform[inBoneIndices.w] * vec4(inPosition, 1.0))  * inBoneWeights.w;

    gl_Position = sceneMatrices.proj * sceneMatrices.view * constants.model * vec4(boneSpace, 1.0);
    fragTexCoord = inTexCoord;
}

