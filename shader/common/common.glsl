#ifndef COMMON_GLSL
#define COMMON_GLSL
#include "shaderStructs.glsl"

vec3 fromLinear(vec3 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linearRGB.rgb * vec3(12.92);

    return mix(higher, lower, cutoff);
}

vec4 fromLinear(vec4 linearRGB)
{
    return vec4(fromLinear(linearRGB.rgb), linearRGB.a);
}

vec3 fromSRGB(vec3 sRGB)
{
    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.0031308 * 12.92));
    vec3 higher = pow(vec3(0.94786729) * (sRGB.rgb + vec3(0.055)), vec3(2.4));
    vec3 lower = sRGB.rgb / vec3(12.92);

    return mix(higher, lower, cutoff);
}

vec4 fromSRGB(vec4 sRGB)
{
    return vec4(fromSRGB(sRGB.xyz), sRGB.a);
}

vec2 sign_not_zero(in vec2 v)
{
	return fma(step(0, v),vec2(2.f),vec2(-1.f));
}

float sign_not_zero(in float v)
{
	return fma(step(0, v),2.f, -1.f);
}

vec2 one_to_minus_one(in vec2 value) 
{
    return fma(value, vec2(2.0f), vec2(-1.f));
}

vec2 zero_to_one(in vec2 value) 
{
    return fma(value, vec2(0.5f), vec2(0.5f));
}

//[-1, 1] domain
vec3 get_octahedral_direction(in vec2 uv) 
{
    vec3 n = vec3(uv.xy, 1.0 - abs(uv.x) - abs(uv.y));
    n.xy = mix((1.0-abs(n.yx)) * sign_not_zero(n.xy), n.xy, step(0.0, n.z));
    return normalize(n);
}
//[-1, 1] domain
vec2 get_octahedral_coords(in vec3 direction)
{
    vec2 uv= direction.xy *  (1.f / dot(abs(direction), vec3(1.f)));
    uv = mix((1.0 - abs(uv.yx)) * sign_not_zero(uv), uv, step(0.0, direction.z));
	return uv;
}

vec3 pack1212(vec2 val) 
{
    vec2 val1212 = floor(val * 4095);
    vec2 high = floor(val1212 / 256);
    vec2 low = val1212 - high * 256;
    vec3 val888 = vec3(low, high.x + high.y * 16);
    return clamp(val888 / 255, 0, 1);
}

vec2 unpack1212(vec3 val)
{
    vec3 val888 = floor(val*255);
    float high = floor(val888.z / 16);
    float low = val888.z - high * 16;
    vec2 val1212 = val888.xy + vec2(low, high) * 256;
    return clamp(val1212 / 4095, 0, 1);
}

//void calculateTBN(inout vec3 normal, inout vec3 tangent, inout vec3 bitangent, vec3 position, vec2 uv)
//{
//	normal = normalize(normal);
//	tangent = cross(dFdy(position), normal) * dFdx(uv.x) + cross(normal, dFdx(position)) * dFdy(uv.x);
//
//	tangent = normalize(tangent - dot(tangent, normal) * normal);
//	bitangent = -normalize(cross(normal, tangent));
//}

void orthonormalize(inout vec3 normal, inout vec3 tangent, out vec3 bitangent) {
    //normal = normalize(normal);
    //tangent = normalize(tangent - dot(normal, tangent) * normal);
    //bitangent = normalize(bitangent - dot(normal, bitangent) * normal - dot(tangent, bitangent) * tangent);
    normal = normalize(normal);
    tangent = normalize(tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = normalize(cross(normal, tangent));
}

vec3 tangentSpaceNormal(in vec2 normalMapSample, in vec3 normal, in vec3 tangent, in vec3 bitangent)
{
    //Currently disabled because it produces bogus
    vec2 convertedNormalMapSample = fma(normalMapSample,vec2(2.0), vec2(-1.0));
    vec3 tangentNormal = vec3(convertedNormalMapSample.xy, sqrt(1.0-convertedNormalMapSample.x*convertedNormalMapSample.x - convertedNormalMapSample.y*convertedNormalMapSample.y));

	mat3 tangentFrame = mat3(tangent , bitangent , normal);
    return tangentFrame * tangentNormal;


    return normalize(normal);
}


vec3 computeTangentSpaceNormal(in vec2 normalMapSample, in vec3 normal, in vec4 tangent)
{
    //Currently disabled because it produces bogus
    vec2 convertedNormalMapSample = fma(normalMapSample,vec2(2.0), vec2(-1.0));
    vec3 tangentNormal = vec3(convertedNormalMapSample.xy, sqrt(1.0-convertedNormalMapSample.x*convertedNormalMapSample.x - convertedNormalMapSample.y*convertedNormalMapSample.y));
    vec3 tmpNormal = normalize(normal);
    vec3 tmpTangent = normalize(tangent.xyz);
//    vec3 a = abs(tmpNormal);
//    uint uyx = (a.x - a.y) < 0? 1 : 0;
//    uint uzx = (a.x - a.z) < 0? 1 : 0;
//    uint uzy = (a.y - a.z) < 0? 1 : 0;
//    uint xm = uyx & uzx;
//    uint ym = (1 ^xm) & uzy;
//    uint zm = 1 ^ ( xm | ym);
//    vec3 tmpTangent = normalize(cross(tmpNormal, vec3(xm, ym, zm)));

    float NdotT = dot(tmpTangent, tmpNormal);
    bool nonParallel = abs(NdotT) < 0.9999f;
    bool nonZero = dot(tmpTangent, tmpTangent) > 0.f;
    bool valid = nonZero && nonParallel && false;
    if(valid) {
        tmpTangent = normalize(tmpTangent - tmpNormal * NdotT);
        vec3 tmpBitangent = cross(tmpNormal, tmpTangent) * tangent.w;
        //return normalize(tmpTangent * tangentNormal.x + tmpBitangent * tangentNormal.y + tmpNormal * tangentNormal.z);
        mat3 tangentFrame = mat3(tmpTangent, tmpBitangent, tmpNormal);
        return normalize( tangentNormal *tangentFrame );
    }
    else {
        return tmpNormal;
    }
}

vec3 quaternion_rotate(vec4 q, vec3 v)
{
    return cross(q.xyz, v) * (2.f * q.w) + 
                v * (q.w * q.w - dot(q.xyz, q.xyz)) + 
                q.xyz * (2.f * dot(v, q.xyz));
}


float hash_vec3(in vec3 seed) {
    //courtesy of Hoskins, Dave https://www.shadertoy.com/view/4djSRW
    vec3 p3  = fract(seed * .1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float max_component(vec2 v) {
	return max(v.x, v.y);
}

float max_component(vec3 v) {
	return max(v.z, max(v.x, v.y));
}

float max_component(vec4 v) {
	return max(v.w, max(v.z, max(v.x, v.y)));
}


#endif