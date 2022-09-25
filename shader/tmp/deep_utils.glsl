/******************************************************************************
**
**  This file is part of Met.3D -- a research environment for the
**  three-dimensional visual exploration of numerical ensemble weather
**  prediction data.
**
**  Copyright 2015-2018 Marc Rautenhaus
**  Copyright 2015      Michael Kern
**  Copyright 2017-2018 Bianca Tost
**
**  Computer Graphics and Visualization Group
**  Technische Universitaet Muenchen, Garching, Germany
**
**  Met.3D is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Met.3D is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Met.3D.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

#extension GL_NV_fragment_shader_interlock : enable
//*************************************************************
//                      Constants
//*************************************************************

const uint arraySize = 6;


const float PI = 3.14159265359;

const mat4x3 lightBiasMatrix = mat4x3(vec3(0.5,0,0),
                                vec3(0,0.5,0),
                                vec3(0,0,0.5),
                                vec3(0.5,0.5,0.5));
//*************************************************************
//                      "Enums"
//*************************************************************



uniform int sampleMode = 0;

const int NEAREST_SAMPLE = 0;
const int BILINEAR_SAMPLE = 1;

uniform int shadowMapType = 0;

const int NOSHADOWMAPTYPE               = 0;
const int BASICSHADOWMAPTYPE            = 1;
const int DEEPSHADOWMAPTYPE             = 2;
const int ADAPTIVEVOLUMETRISHADOWMAPTYPE= 3;
const int UNCOMPRESSEDSHADOWMAPTYPE     = 4;
const int MOMENTSHADOWMAPTYPE           = 5;
const int STOCHASTICSHADOWMAPTYPE       = 6;

// Deep Shadow Map
struct Entry {
    uint depthTransmittance;
    //float depth;
    //float transmittance;
    int next;
};


uniform float compressionError;
uniform vec3 lightDir;
uniform mat4x4 lightViewMatrix;
uniform mat4x4 lightViewProjection;

#ifndef QUANTIZED
#define QUANTIZED
uniform bool isQuantizedMoments;
uniform bool isMBOIT;
#endif
uniform bool avsmHalfPrecision;
uniform vec3 shadowColour;
uniform float overestimate;
uniform int multisamples = 1;

uniform int filterKernelSize;

uniform bool    stochasticShadowMode;

uniform sampler2D shadowMap;
uniform sampler2D momentsTexture;
uniform sampler2D opticalDepthTexture;
uniform float depthBias;
//uniform int arraySize;
//Assumed normalized
uniform uvec2 dimensions;

layout(std430, binding = 0) buffer DeepShadowTexture
{
    int gridArray[];
} deepShadowTexture;
layout(std430, binding = 1) buffer DeepShadowList
{
    Entry entries[];
} deepShadowList;
layout(std430, binding = 2) buffer DeepShadowCounter
{
    uint counter;
} deepShadowCounter;
layout(std430, binding = 3) buffer DeepShadowMipOffsets
{
    int offset[];
} deepShadowMipOffsets;
layout(std430, binding = 4) buffer DeepShadowFilterValues
{
    float values[];
} deepShadowFilterValues;
layout(std430, binding = 0) buffer AVSMBuffer
{
    uint vals[];
} avsmBuffer;
layout(std430, binding = 0) buffer AVSMBuffer32
{
    vec2 vals[];
} avsmBuffer32;

// Helper Functions
bool deeper(float a, float b) {
    return a > b;
}
int UVtoIndex(in vec2 uv) {
    uvec2 gridPosition = uvec2(fract(uv)*vec2(dimensions));
    return int(gridPosition.x + gridPosition.y*dimensions.x);
}
int UVtoIndex(in vec2 uv, in int mipLevel) {
    uvec2 gridPosition = uvec2(fract(uv)*vec2(dimensions));
    return int(gridPosition.x + gridPosition.y*dimensions.x+deepShadowMipOffsets.offset[mipLevel]);
}
vec3 toLightSpace(in vec3 pos) {
    vec4 temp = lightViewProjection * vec4(pos,1);
    temp.xyz /= temp.w;
    temp.w = 1;
    //Convert [-1;1] -> [0;1]
    temp.xyz = lightBiasMatrix* temp;
    //Clamp
    temp.xy = fract(temp.xy);
    temp.z = clamp(temp.z, 0.0, 1.0);
    return temp.xyz;
}

//*************************************************************
//Deep Shadow Maps
//*************************************************************
void insertDeepShadowMap(in int gridIndex, in float depth, in float alpha) {
    //"Allocate" new Entry, offset by one full image
    uint newIndex = atomicAdd(deepShadowCounter.counter, 1);
    //SSBO full
    if(newIndex >= deepShadowList.entries.length()) {
        return;
    }
    int index = deepShadowTexture.gridArray[gridIndex];



    int oldIndex = atomicExchange(deepShadowTexture.gridArray[gridIndex], int(newIndex));
    //insert new Entry 
    Entry entry;
    entry.depthTransmittance = packUnorm2x16(vec2(depth,alpha));
    entry.next = oldIndex;
    deepShadowList.entries[newIndex] = entry;
}
void insertDeepShadowMapMipMapped(in vec2 uv, in int mipLevel, in float depth, in float alpha) {
    int gridIndex = UVtoIndex(uv,mipLevel);
    insertDeepShadowMap(gridIndex, depth, alpha);
}
void insertDeepShadowMap(in vec2 uv, in float depth, in float alpha) {
    int gridIndex = UVtoIndex(uv);
    insertDeepShadowMap(gridIndex, depth, alpha);
}
void insertDeepShadowMap(in vec3 pos, in float alpha) {
    vec3 lightspace = toLightSpace(pos);
    insertDeepShadowMap(lightspace.xy, lightspace.z, alpha);
}
float fetchDeepShadowMap(in int index, in float sampleDepth) {
    if(index == -1) {
        return 1.;
    }
    Entry current = deepShadowList.entries[index];
    vec2 currentDepthTransmittance = unpackUnorm2x16(current.depthTransmittance);
    if(!deeper(sampleDepth, currentDepthTransmittance.x)) {
        return 1.;
    }
    if(current.next != -1) {
        Entry next = deepShadowList.entries[current.next];
        vec2 nextDepthTransmittance = unpackUnorm2x16(next.depthTransmittance);
        while(deeper(sampleDepth, nextDepthTransmittance.x)){
            if(next.next == -1) {
                return nextDepthTransmittance.y;
            }
            current = next;
            currentDepthTransmittance = nextDepthTransmittance;
            next = deepShadowList.entries[next.next];
            nextDepthTransmittance = unpackUnorm2x16(next.depthTransmittance);
        }
        float deltaZ = (sampleDepth - currentDepthTransmittance.x);
        float weight = deltaZ/(nextDepthTransmittance.x-currentDepthTransmittance.x);
        //Lerp between the two entries' transmittances

        return mix(currentDepthTransmittance.y, nextDepthTransmittance.y, weight);
    } else {
        return currentDepthTransmittance.y;
    }
}
float fetchDeepShadowMap(in vec2 uv, in float sampleDepth) {
    int index = deepShadowTexture.gridArray[UVtoIndex(uv)];
    return fetchDeepShadowMap(index, sampleDepth);
}
float fetchDeepShadowMapMipMapped(in vec2 uv, in float sampleDepth, in int mipMapLevel) {
    int index = deepShadowTexture.gridArray[UVtoIndex(uv, mipMapLevel)];
    return fetchDeepShadowMap(index, sampleDepth);
}
float sampleDeepShadowMap(in vec2 uv,in float depth) {
    if(sampleMode == NEAREST_SAMPLE) {
        return fetchDeepShadowMap(uv,depth);
    } else if (sampleMode == BILINEAR_SAMPLE) {
        float total = 0.0;
        vec2 texel = 1.0/ vec2(dimensions);
        uv = uv - 0.499*texel;
        vec4 samples = vec4(fetchDeepShadowMap(uv+vec2(0,texel.y), depth),
                        fetchDeepShadowMap(uv+vec2(texel.x,0), depth),
                        fetchDeepShadowMap(uv+texel, depth),
                        fetchDeepShadowMap(uv, depth));
        vec2 fracts = fract(uv*dimensions);
        float upper = mix(samples.x, samples.z, fracts.x);
        float lower = mix(samples.w, samples.y, fracts.x);

        return mix(lower, upper, fracts.y);
    } else {
        return fetchDeepShadowMap(uv,depth);
    }
}
//********************************************************************************
//Adaptive Volumetric Shadow Maps
//********************************************************************************

void insertAdaptiveVolumetricShadowMap16(in uint index, in float depth, in float iAlpha) {
    float prevTransmittance = 1.0;
    float prevDepth = 0.0;
    uint inserted = 0;
    vec2 b[arraySize+1];

    for(uint i = 0; i < arraySize; i++) {
        //empty entry
        vec2 temp = unpackUnorm2x16(avsmBuffer.vals[index*arraySize+i]);
        if(!deeper(depth, temp.x)) {
            if(inserted == 0) {
                inserted = 1;
                float deltaX = (temp.x-depth)/(temp.x-prevDepth);
                //Lerp between the two entries' transmittances
                float interpolatedTransmittance = mix(prevTransmittance, temp.y, deltaX);
                b[i] = vec2(depth, interpolatedTransmittance* iAlpha);
            }
            temp.y *= iAlpha;
        } else {
            prevTransmittance = temp.y;
            prevDepth = temp.x;
        }
        b[i+inserted] = temp;
    }
    if(inserted == 0) {
        b[arraySize] = vec2(depth, prevTransmittance* iAlpha);
    }

    //1/0 is apparently guaranteed to result in +inf
    //https://stackoverflow.com/a/33961589
    float min_e     =   1./0.;
    uint arg_min_e   = 0;
    for(uint i = 1; i < arraySize; i++) {
        float deltaT = abs( (b[i+1].x-b[i-1].x) *(b[i+1].y-b[i].y)
                       -    (b[i+1].x-b[i].x)   *(b[i+1].y-b[i-1].y));
        if(deltaT < min_e) {
            arg_min_e = i;
            min_e = deltaT;
        }
    }
    for(uint i = 0; i < arg_min_e;i++) {
        avsmBuffer.vals[index*arraySize+i] = packUnorm2x16(b[i]);
    }
    for(uint i = arg_min_e; i < arraySize; i++) {
        avsmBuffer.vals[index*arraySize+i]= packUnorm2x16(b[i+1]);
    }
}
void insertAdaptiveVolumetricShadowMap16(in vec3 pos, in float iAlpha) {
    vec3 lightspace = toLightSpace(pos);
    uint gridIndex = uint(UVtoIndex(lightspace.xy));
    insertAdaptiveVolumetricShadowMap16(gridIndex,lightspace.z, iAlpha);
}
float fetchAVSM16(in vec2 uv, float depth) {
    uint index = UVtoIndex(uv);
    uint first = (index*arraySize);
    uint last = ((1+index)*arraySize)-1;
    vec2 prev = unpackUnorm2x16(avsmBuffer.vals[first]);
    if(!deeper(depth, prev.x)) {
        return 1.0;
    }
    for(uint i = first+1; i <= last; i++) {
        vec2 current = unpackUnorm2x16(avsmBuffer.vals[i]);
        if(!deeper(depth, current.x)) {
            float weight = (current.x-depth)/(current.x-prev.x);
            //Lerp between the two entries' transmittances
            return mix(prev.y, current.y, weight);
            //return prev.y;
        }
        prev = current;
    }
    return prev.y;
}

void insertAdaptiveVolumetricShadowMap32(in uint index, in float depth, in float iAlpha) {
    float prevTransmittance = 1.0;
    float prevDepth = 0.0;
    uint inserted = 0;
    vec2 b[arraySize+1];

    for(uint i = 0; i < arraySize; i++) {
        //empty entry

        vec2 temp = avsmBuffer32.vals[index*arraySize+i];
        if(!deeper(depth, temp.x)) {
            if(inserted == 0) {
                inserted = 1;
                float deltaX = (temp.x-depth)/(temp.x-prevDepth);
                //Lerp between the two entries' transmittances
                float interpolatedTransmittance = mix(prevTransmittance, temp.y, deltaX);
                b[i] = vec2(depth, interpolatedTransmittance* iAlpha);
            }
            temp.y *= iAlpha;
        } else {
            prevTransmittance = temp.y;
            prevDepth = temp.x;
        }
        b[i+inserted] = temp;
    }
    if(inserted == 0) {
        b[arraySize] = vec2(depth, prevTransmittance* iAlpha);
    }
    //1/0 is apparently guaranteed to result in +inf
    //https://stackoverflow.com/a/33961589
    float min_e     =   1./0.;
    uint arg_min_e   = 0;
    for(uint i = 1; i < arraySize; i++) {
        float deltaT = abs( (b[i+1].x-b[i-1].x) *(b[i+1].y-b[i].y)
                       -    (b[i+1].x-b[i].x)   *(b[i+1].y-b[i-1].y));
        if(deltaT < min_e) {
            arg_min_e = i;
            min_e = deltaT;
        }
    }
    for(uint i = 0; i < arg_min_e;i++) {
        avsmBuffer32.vals[index*arraySize+i] = b[i];
    }
    for(uint i = arg_min_e; i < arraySize; i++) {
        avsmBuffer32.vals[index*arraySize+i]= b[i+1];
    }
}
void insertAdaptiveVolumetricShadowMap32(in vec3 pos, in float iAlpha) {
    vec3 lightspace = toLightSpace(pos);
    uint index = uint(UVtoIndex(lightspace.xy));
    insertAdaptiveVolumetricShadowMap32(index, lightspace.z, iAlpha);
}


float fetchAVSM32(in vec2 uv, float depth) {
    uint index = UVtoIndex(uv);
    uint first = (index*arraySize);
    uint last = ((1+index)*arraySize)-1;
    vec2 prev = avsmBuffer32.vals[first];
    if(!deeper(depth, prev.x)) {
        return 1.0;
    }
    for(uint i = first+1; i <= last; i++) {
        vec2 current = avsmBuffer32.vals[i];
        if(!deeper(depth, current.x)) {
            float weight = (current.x-depth)/(current.x-prev.x);
            //Lerp between the two entries' transmittances
            return mix(prev.y, current.y, weight);
            //return prev.y;
        }
        prev = current;
    }
    vec2 temp = avsmBuffer32.vals[last];
    return temp.y;
}
float fetchAVSM(in vec2 uv, float depth) {
    if(avsmHalfPrecision) {
        return fetchAVSM16(uv, depth);
    } else {
        return fetchAVSM32(uv, depth);
    }
}
void insertAdaptiveVolumetricShadowMap(in vec3 pos, in float iAlpha) {
    if(avsmHalfPrecision) {
        insertAdaptiveVolumetricShadowMap16(pos, iAlpha);
    } else {
        insertAdaptiveVolumetricShadowMap32(pos, iAlpha);
    }
}
void insertAdaptiveVolumetricShadowMap(in uint index, in float depth, in float iAlpha) {
    if(avsmHalfPrecision) {
        insertAdaptiveVolumetricShadowMap16(index, depth, iAlpha);
    } else {
        insertAdaptiveVolumetricShadowMap32(index, depth, iAlpha);
    }
}
float sampleAdaptiveVolumetricShadowMap(in vec2 uv, in float depth) {
    if(sampleMode == NEAREST_SAMPLE) {
        return fetchAVSM(uv, depth);
    }
    else if(sampleMode == BILINEAR_SAMPLE) {
        vec2 weights = fract(uv*vec2(dimensions));
        vec2 texelSize = 1.f/vec2(dimensions);
        float x1y1 = fetchAVSM(uv, depth);
        float x2y1 = fetchAVSM(uv+vec2(texelSize.x,0), depth);
        float x1y2 = fetchAVSM(uv+vec2(0,texelSize.y), depth);
        float x2y2 = fetchAVSM(uv+texelSize, depth);
        float x1 = mix(x1y1, x1y2, weights.y);
        float x2 = mix(x2y1, x2y2, weights.y);
        return mix(x1, x2, weights.x);
    }
    return 1.f;
}
//********************************************************************************
//Moment Shadow Maps
//********************************************************************************
/*
const mat2x2 quantizationTransform[2] = mat2x2[](mat2x2(vec2(1.5f, 0.8660254f),
                                               vec2(-2.f, -0.384900182f)),
                                               mat2x2(vec2( 4.f, 0.5f),
                                               vec2(-4.f, 0.5f)));
                                               */
const mat2x2 inverseQuantizationTransform[2] = mat2x2[](mat2x2( vec2(-1.0f/3.0f, -0.75f),
                                                                vec2(sqrt(3.0f),0.75f*sqrt(3.0f))),
                                                        mat2x2( vec2(0.125f,1.0f ),
                                                                vec2(-0.125f , 1.0f)));
vec4 undoQuantization(vec4 quantizedMoments) {
    vec4 temp;
    temp.xz = inverseQuantizationTransform[0]*(quantizedMoments.xz-0.5f);
    temp.yw = inverseQuantizationTransform[1]*quantizedMoments.yw;
    //Apply bias to avoid sampled Moments outside of the convex hull
    const float MomentBias = 6.0e-5f;
    return mix(temp, vec4(0.0f, 0.628f, 0.0f, 0.628f), MomentBias);
}
vec4 biasMoments(vec4 moments) {
    const float MomentBias = 3.0e-7f;
    return mix(moments, vec4(0.0f, 0.375f, 0.0f, 0.375f), MomentBias);
}

//[Peters et al. 2015]
float calculateMSMShadowIntensity(vec4 b, float depth) {
    float L32D22 = -b.x* b.y+ b.z;
    float D22 = -b.x* b.x+ b.y;
    float SquaredDepthVariance = -b.y* b.y+ b.w;
    float D33D22 = dot(vec2(SquaredDepthVariance, -L32D22),
                       vec2(D22                 ,  L32D22));
    float InvD22 = 1.f/D22;
    float L32= L32D22*InvD22;
    float D33= D33D22*InvD22;
    float InvD33 = 1.f/D33;
    vec3 z;
    z.x = depth;
    vec3 c = vec3(1.f,z.x,z.x*z.x);
    c.y -=b.x;
    c.z -=b.y+L32*c.y;
    c.y *=InvD22;
    c.z *=InvD33;
    c.y -=L32*c.z;
    c.x -=dot(c.yz, b.xy);
    float InvC2=1.f/c.z;
    float p=c.y*InvC2;
    float q=c.x*InvC2;
    float r=sqrt((p*p*0.25f)-q);
    z.y =-p*0.5f-r;
    z.z =-p*0.5f+r;
    vec4 Switch =
            (z.z<z.x)?vec4(z.y,z.x, 1.0f, 1.0f): (
            (z.y<z.x)?vec4(z.x,z.y, 0.0f, 1.0f):
            vec4(0.0f,0.0f, 0.0f, 0.0f));
    float Quotient= (Switch.x*z.z-b.x*(Switch.x+z.z)+b.y)
            /((z.z-Switch.y)*(z.x-z.y));
    return 1.-clamp(Switch.z+Switch.w*Quotient,0.f,1.f);
}
//[Muenstermann 2018]
float calculateMSMShadowTransparency(vec4 b, float depth, float b_0) {
    float L32D22 = -b.x* b.y+ b.z;
    float D22 = -b.x* b.x+ b.y;
    float SquaredDepthVariance = -b.y* b.y+ b.w;
    float InvD22 = 1/D22;
    float L32= L32D22*InvD22;
    float D33= -L32D22*L32 + SquaredDepthVariance;

    vec3 z;
    z.x = depth;
    vec3 c = vec3(1.f,z.x,z.x*z.x);
    c.y -=b.x;
    c.z -=b.y+L32*c.y;
    c.y *=InvD22;
    c.z /=D33;
    c.y -=L32*c.z;
    c.x -=dot(c.yz, b.xy);
    float InvC2=1.f/c.z;
    float p=c.y*InvC2;
    float q=c.x*InvC2;
    float r=sqrt((p*p*0.25f)-q);
    z.y =-p*0.5f-r;
    z.z =-p*0.5f+r;

    vec3 polynomial;
    float f0 = overestimate;
    float f1 = z.y < z.x?1.0f : 0.0f;
    float f2 = z.z < z.x?1.0f : 0.0f;
    float f01 = (f1-f0)/(z.y-z.x);
    float f12 = (f2-f1)/(z.z-z.y);
    float f012 = (f12-f01)/(z.z -z.x);
    polynomial.x = f01-f012*z.y;
    polynomial.z = f012;
    polynomial.y = polynomial.x - f012*z.x;
    polynomial.x = f0-polynomial.x*z.x;
    float reconstructedOpticalDepth = polynomial.x + dot(b.xy, polynomial.yz);
    float temp = clamp(exp(-b_0*reconstructedOpticalDepth),0.0f, 1.0f);
    return temp;
}
float sampleMoments(vec2 uv, float depth) {
    vec4 moments = texture(momentsTexture, uv.xy);
    if(isQuantizedMoments) {
        return calculateMSMShadowIntensity(undoQuantization(moments), depth*2.0-1.0);
    } else {
        return calculateMSMShadowIntensity(biasMoments(moments), depth*2.0-1.0);
    }
}
float sampleMomentsMBOIT(vec2 uv, float depth) {


    float b_0 = texture(opticalDepthTexture, uv.xy).r;
    if(b_0 < 0.001) return 1.0;
    vec4 moments= texture(momentsTexture, uv.xy);
    //b_0 holds the total optical Depth: optical Depth = - ln (Transmittance)
    float totalTransmittance = exp(-b_0); //Not sure what to do with this
    moments/= b_0;

    return calculateMSMShadowTransparency(biasMoments(moments), depth*2.0-1.0, b_0);
}
/*
void calculateMoments(in float depth, out vec4 moments) {
    //remap [0;1] -> [-1;1] [Peters et al. 2017]
    //Proves to be more robust when stored in 16 bits
    float depthSNorm = (depth*2.0f)-1.0f;
    float depthSquared = depthSNorm*depthSNorm;

    moments = vec4(depthSNorm, depthSquared, depthSquared*depthSNorm, depthSquared*depthSquared);
    moments.xz = quantizationTransform[0]*moments.xz+0.5f;
    moments.yw = quantizationTransform[1]*moments.yw;
}
*/
//Stochastic Shadow utils

float generateRandomNumber(in vec3 seed) {
    //courtesy of Hoskins, Dave https://www.shadertoy.com/view/4djSRW
    vec3 p3  = fract(seed * .1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
int calculateMask(vec3 seed, float alpha) {
    int resultingMask = 0x0;
    for(int i = 0; i <multisamples; i++ ){
        float randomNumber = generateRandomNumber(seed*i);
        if(randomNumber <alpha) {
            resultingMask |= (1 << i);
        }
    }
    return resultingMask;
}
float fetchBasicShadowMap(in vec2 uv, in float depth) {
    return texture(shadowMap, uv).r > depth? 1.0f: 0.0f;
}
vec4 fetch4BasicShadowMap(inout vec2 uv, in float depth) {
    //Todo replace with gather4
    vec2 texel = 1.0/ vec2(dimensions);
    uv = uv - 0.499*texel;
    vec4 samples = vec4(fetchBasicShadowMap(uv+vec2(0,texel.y), depth),
                    fetchBasicShadowMap(uv+vec2(texel.x,0), depth),
                    fetchBasicShadowMap(uv+texel, depth),
                    fetchBasicShadowMap(uv, depth));
    return samples;
}
float sampleBasicShadowMap(in vec2 uv, in float depth) {
    if(sampleMode == NEAREST_SAMPLE) {
        return fetchBasicShadowMap(uv,depth);
    } else if (sampleMode == BILINEAR_SAMPLE) {
        float total = 0.0;
        vec4 samples = fetch4BasicShadowMap(uv, depth);
        vec2 fracts = fract(uv*dimensions);
        float upper = mix(samples.x, samples.z, fracts.x);
        float lower = mix(samples.w, samples.y, fracts.x);

        return mix(lower, upper, fracts.y);
    }
    return 1.0f;
}

// PCFshadowMap
float samplePCFShadowMap(in vec2 uv,in float depth) {
    float sum = 0.0f;
    vec2 texel = 1.0/ vec2(dimensions);
    int corner = filterKernelSize>>1;
    for(int i = -corner; i <= corner; ++i) {
        for(int j = -corner; j <= corner; ++j) {
            sum += sampleBasicShadowMap(uv+vec2(i,j)*texel, depth);
        }
    }
    return sum/(filterKernelSize*filterKernelSize);
}
//********************************************************************************
//Interface functions
//********************************************************************************
void insertShadowMapIndex(in int gridIndex, in float depth, in float alpha) {
    if(shadowMapType == DEEPSHADOWMAPTYPE
        || shadowMapType == UNCOMPRESSEDSHADOWMAPTYPE) {
        insertDeepShadowMap(gridIndex, depth, alpha);
    } else if(shadowMapType == ADAPTIVEVOLUMETRISHADOWMAPTYPE) {
        insertAdaptiveVolumetricShadowMap(gridIndex, depth, alpha);
    }
}
void insertShadowMapWorldPos(in vec3 pos, in float alpha) {
    if(shadowMapType == DEEPSHADOWMAPTYPE
            || shadowMapType == UNCOMPRESSEDSHADOWMAPTYPE) {
        insertDeepShadowMap(pos, alpha);
    } else if(shadowMapType == ADAPTIVEVOLUMETRISHADOWMAPTYPE) {
        insertAdaptiveVolumetricShadowMap(pos, alpha);
    }
}
float sampleShadowMap(in vec2 uv, in float depth) {
    if(shadowMapType == BASICSHADOWMAPTYPE) {

        return samplePCFShadowMap(uv, depth);
    } else if(shadowMapType == DEEPSHADOWMAPTYPE
            || shadowMapType == UNCOMPRESSEDSHADOWMAPTYPE) {
        return sampleDeepShadowMap(uv, depth);
    } else  if(shadowMapType == ADAPTIVEVOLUMETRISHADOWMAPTYPE) {
        return sampleAdaptiveVolumetricShadowMap(uv, depth);
    } else if(shadowMapType == MOMENTSHADOWMAPTYPE) {
        if(isMBOIT){
            return sampleMomentsMBOIT(uv, depth);
        }else {
            return sampleMoments(uv, depth);
        }
    } else if(shadowMapType == STOCHASTICSHADOWMAPTYPE) {
        return samplePCFShadowMap(uv, depth);
    }
    return 1.0;
}
float sampleShadowMap(in vec3 pos) {
    vec3 lightSpace = toLightSpace(pos);
    return sampleShadowMap(lightSpace.xy, lightSpace.z);
}
float sampleShadowMap(in vec3 pos, in vec3 normal) {
    vec3 lightSpace = toLightSpace(pos);
    lightSpace.z -= dot(normal, lightDir)*depthBias;
    return sampleShadowMap(lightSpace.xy, lightSpace.z);
}
void insertSurface(in int index,in float depth, in float alpha) {
    //beginInvocationInterlockNV();
    //endInvocationInterlockNV();
    insertShadowMapIndex(index, depth,1);
    insertShadowMapIndex(index, depth+(1.0f/float(1<<16)),alpha);
}
void insertSurface(in vec2 uv,in float depth, in float alpha) {
    int index = UVtoIndex(uv);
    insertSurface(index, depth, alpha);
}
void insertSurface(in vec3 pos, in float alpha) {
    vec3 lightspace = toLightSpace(pos);
    int index = UVtoIndex(lightspace.xy);
    insertSurface(index, lightspace.z, alpha);
}
