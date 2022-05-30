#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"

layout(std430, push_constant) uniform PushConstants
{
	uint sceneBinding;
    uint albedoBinding;
    uint albedoSampler;
    uint normalBinding;
    uint normalSampler;
    uint pbrBinding;
    uint pbrSampler;
    uint depthBinding;
    uint depthSampler;
    uint stencilBinding;
    uint stencilSampler;
} constants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;

float intPow5(float val) {
    float tmp = val * val;
    tmp = tmp * tmp;
    return tmp * val;
}

vec3 F_sphericalGaussian(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * exp2((-5.55473 * cosTheta -  6.98316) * cosTheta);
}

float lambda_smith(float cosTheta, float alpha) {
    float cosThetaSqr = cosTheta * cosTheta;
    return (-1.0 + sqrt(1.0 + alpha * (1.0 - cosThetaSqr) / cosThetaSqr)) * 0.5;
}

float NDF_GGX(float NdotH, float alpha) {
    float NdotHSqr = NdotH * NdotH;
    float t = NdotHSqr * alpha - NdotHSqr + 1.0;
    return alpha / (3.1415926 * t * t);
}

float G2_smith(float NdotL, float NdotV, float alpha) {
    float lambdaL = lambda_smith(NdotL, alpha);
    float lambdaV = lambda_smith(NdotV, alpha);
    return 1.0 / (1.0 + lambdaV + lambdaL);
}

//[E. Hammon, 2017] "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
vec3 brdf_hammon_diffuse(float NdotL, float NdotV,float NdotH, float VdotL, float VdotH, vec3 diffuseColor, float alpha)
{

    float facing = 0.5 + 0.5 * VdotL;

    float energyBias = mix(0.0, 0.5, alpha);
    float f90 = energyBias + 2.0 * VdotH * VdotH * alpha;
    float fNdotL = 1.0 - (0.04 + (f90 - 0.04) * exp2((-5.55473 * NdotL - 6.98316) *NdotL));
    float fNdotV = 1.0 - (0.04 + (f90 - 0.04) * exp2((-5.55473 * NdotV - 6.98316) *NdotV));

    float surfaceRough = NdotH <= 0.0 ? 0.0 : facing * (0.9 - 0.4 * facing) * (0.5 + NdotH) / NdotH;
    float surfaceSmooth = 1.008 * (fNdotL * fNdotV);
    //float smoothV = 1.05 * (1-intPow5(1-NdotL)) * (1- intPow5(1-NdotV));
    float single = 1 / 3.1415926 * mix(surfaceSmooth, surfaceRough, alpha);
    float multi = 0.1159 * alpha;

    return diffuseColor.xyz * (single + diffuseColor.xyz * multi);
}
vec3 brdf_lambert(float NdotL, float NdotV,float NdotH, float VdotL, float VdotH, vec3 diffuseColor, float alpha)
{
    float single = 1 / 3.1415926;

    return  diffuseColor.xyz * single;
}

vec3 brdf_cook_torrance_specular(float NdotL, float NdotV, float NdotH, float LdotH, vec3 specularColor, float alpha) {
    float NDF = NDF_GGX(NdotH, alpha);
    
    float G2 = G2_smith(NdotL, NdotV, alpha);

    vec3 F = F_sphericalGaussian(LdotH, specularColor);

    return ((NDF * G2) * F) / max(4.0  * NdotV, 1e-4); //* NdotL Take out NdotL
}

void main() {
//    vec3 normal = subpassLoad(inputNormal).xyz *2.0 - 1.0;
//    vec3 ambient = 0.1 * constants.lightColor.xyz;
//    float diff = max(dot(normal, constants.lightDir.xyz), 0.0);
//    vec3 diffuse = diff * constants.lightColor.xyz;
//
    Scene scene = scenes[constants.sceneBinding].scene;
	DirectionalLight light1 = scene.dirLight1;
	DirectionalLight light2 = scene.dirLight2;
    
    uint stencil = texelFetch( usampler2D(utextures2D[constants.stencilBinding], samplers[constants.stencilSampler]), ivec2(gl_FragCoord.xy), 0).x;

    if(stencil == 0) {
        discard;
    } else {
    vec3 normal = texture(sampler2D(textures2D[constants.normalBinding], samplers[constants.normalSampler]), inTexCoord).xyz * 2.0 - 1.0;
    vec4 pbr = texture(sampler2D(textures2D[constants.pbrBinding], samplers[constants.pbrSampler]), inTexCoord);
    vec4 albedo = texture(sampler2D(textures2D[constants.albedoBinding], samplers[constants.albedoSampler]), inTexCoord);
    vec3 depth = texture(sampler2D(textures2D[constants.depthBinding], samplers[constants.depthBinding]), inTexCoord).xyz;

    
    float metalness = pbr.x;
    float alpha = pbr.y * pbr.y;
    vec4 clipSpacePos = vec4(inTexCoord * 2.0 - 1.0, depth.x, 1.0);
    vec4 viewSpacePos = scene.invProj * clipSpacePos;
    viewSpacePos /= viewSpacePos.w;
    vec4 worldSpacePos = scene.invView * viewSpacePos;
    vec3 viewPos = vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
    vec3 viewVec = normalize(viewPos - worldSpacePos.xyz);

    float NdotL = max(dot(normal, light1.dir), 0.0);
    float LdotV = dot(light1.dir, viewVec);
    float NdotV = max(dot(normal, viewVec), 0.0);
    float rcpLenLV = inversesqrt(2 + 2 * LdotV);
    float NdotH = (NdotL + NdotV) * rcpLenLV;
    float LdotH = rcpLenLV * LdotV + rcpLenLV;


    vec3 diffuseColor = (1- metalness) * albedo.xyz;
    vec3 diffuse = brdf_hammon_diffuse(NdotL, NdotV, NdotH, LdotV, LdotH, diffuseColor, alpha) * light1.intensity * light1.color * NdotL;
    vec3 F0 = vec3(0.04, 0.04, 0.04);
    vec3 specularColor = mix(F0 , albedo.xyz,metalness);
    vec3 specular = brdf_cook_torrance_specular(NdotL, NdotV, NdotH, LdotH, specularColor, alpha) * light1.intensity * light1.color;// * NdotL already canceled out

    //outSpecular = texture(sampler2D(textures2D[constants.albedoBinding], samplers[constants.albedoSampler]), inTexCoord);
    outSpecular = vec4(specular, 1);
    outDiffuse = vec4(diffuse,1);
    }
    //outDiffuse = vec4(albedo.xyz * NdotL,1);
    //outDiffuse = vec4(LdotV, NdotL, NdotV,1);
    //outDiffuse = vec4(light1.dir,1);
    //outDiffuse = vec4(normal.xyz * 0.5 + 0.5,1);
    
    //outColor = vec4(subpassLoad(inputColor).xyz * (diffuse + ambient), 1.0);
    //outColor = subpassLoad(inputNormal);

    //float depth = pow(subpassLoad(inputDepth).x, 100);
    //outColor = vec4(depth.xxx , 1.0);
    //outColor = vec4(texture(texSampler, inTexCoord).xxx, 1.0);
    // outColor = vec4(gl_FragCoord.xy / vec2(1920, 1080), 0.0, 1.0);
}