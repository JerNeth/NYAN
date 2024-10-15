#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_debug_printf : enable

//layout(binding = 0) uniform UniformBufferObject {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} ubo;

//layout(std430, push_constant) uniform PushConstants
//{
//    mat4 model;
//} constants;

//layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;
//layout(location = 2) in vec2 inTexCoord;

//layout(location = 0) out vec3 fragColor;
//layout(location = 1) out vec2 fragTexCoord;

void main() {

    vec2 outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f + -1.0f, 0.0f, 1.0f);

//    const vec3 vertices[3] = vec3[3](vec3(0,0,0), vec3(0,1,0), vec3(1,0,0));
//     
//    gl_Position.xyz = vertices[gl_VertexIndex%3];
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    //gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    //fragColor = inColor;
    //fragTexCoord = inTexCoord;
    //debugPrintfEXT("V%d: (%f,%f,%f)", gl_VertexIndex, gl_Position.x, gl_Position.y, gl_Position.z);
}

