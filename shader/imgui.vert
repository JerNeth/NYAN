#version 450 core
#extension GL_KHR_vulkan_glsl:enable
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

//layout(set = 0, binding = 0) buffer ssbos[];
//layout(set = 0, binding = 1) uniform ubos[];
//layout(set = 0, binding = 2) uniform sampler samplers[];
//layout(set = 0, binding = 3) uniform texture2D textures[];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];

layout(push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
    int texId;
    int samplerId;
} pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}