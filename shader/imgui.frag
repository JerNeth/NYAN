#version 450 core
#extension GL_EXT_nonuniform_qualifier:require
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) out vec4 fColor;

//layout(set = 0, binding = 0) buffer ssbos[];
//layout(set = 0, binding = 1) uniform ubos[];
layout(set = 0, binding = 2) uniform sampler samplers[];
layout(set = 0, binding = 3) uniform texture2D textures[];
layout(set = 0, binding = 3) uniform texture2DArray texturesArray[];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];

layout(std430, push_constant) uniform uPushConstant {
    float uScaleX;
    float uScaleY;
    float uTranslateX;
    float uTranslateY;
    int texId;
    int samplerId;
    int customTexId;
    int customSamplerId;
    int arrayLayer;
} pc;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
//    if(customTexId != -1) {
//    fColor = fromLinear(In.Color * texture(sampler2D(textures[pc.customTexId], samplers[pc.customSamplerId]), In.UV.st));
//    } else {
    if(pc.arrayLayer != -1) {
        fColor = fromLinear(In.Color * texture(sampler2DArray(texturesArray[pc.texId], samplers[pc.samplerId]), vec3(In.UV.st, pc.arrayLayer)));
    } else {
        fColor = fromLinear(In.Color * texture(sampler2D(textures[pc.texId], samplers[pc.samplerId]), In.UV.st));
    }
    //}
    //fColor = In.Color * texture(sampler2D(textures[0], samplers[4]), In.UV.st);
}