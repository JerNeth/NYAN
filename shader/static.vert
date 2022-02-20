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

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = sceneMatrices.proj * sceneMatrices.view * constants.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}

