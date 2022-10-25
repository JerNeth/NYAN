#ifndef MOMENTS_GLSL
#define MOMENTS_GLSL

//********************************************************************************
//Moment Shadow Maps
//********************************************************************************

const mat2x2 quantizationTransform[2] = mat2x2[](mat2x2(vec2(1.5f, 0.8660254f),
                                               vec2(-2.f, -0.384900182f)),
                                               mat2x2(vec2( 4.f, 0.5f),
                                               vec2(-4.f, 0.5f)));
                                               
const mat2x2 inverseQuantizationTransform[2] = mat2x2[](mat2x2( vec2(-1.0f/3.0f, -0.75f),
                                                                vec2(sqrt(3.0f),0.75f*sqrt(3.0f))),
                                                        mat2x2( vec2(0.125f,1.0f ),
                                                                vec2(-0.125f , 1.0f)));

                                                                
vec4 quantization(vec4 moments) {
    moments.xz = quantizationTransform[0]*moments.xz+0.5f;
    moments.yw = quantizationTransform[1]*moments.yw;
    return moments;
}

//void calculateMoments4Quant(in float depth, out vec4 moments) {
//    float depthSquared = depth*depth;
//
//    moments = vec4(depth, depthSquared, depthSquared*depth, depthSquared*depthSquared);
//    quantMoments(moments);
//}
//void calculateMoments4QuantSNorm(in float depth, out vec4 moments) {
//    //remap [0;1] -> [-1;1] [Peters et al. 2017]
//    //Proved to be more robust when stored in 16 bits
//    float depthSNorm = (depth*2.0f)-1.0f;
//    calculateMoments4Quant(depthSNorm, moments);
//}

vec4 undo_quantization(in vec4 quantizedMoments) {
    vec4 temp;
    temp.xz = inverseQuantizationTransform[0]*(quantizedMoments.xz-0.5f);
    temp.yw = inverseQuantizationTransform[1]*quantizedMoments.yw;
    //Apply bias to avoid sampled Moments outside of the convex hull
    const float MomentBias = 6.0e-5f;
    return mix(temp, vec4(0.0f, 0.628f, 0.0f, 0.628f), MomentBias);
}
vec4 bias_moments(in vec4 moments) {
    const float MomentBias = 3.0e-7f;
    return mix(moments, vec4(0.0f, 0.375f, 0.0f, 0.375f), MomentBias);
}

//[Peters et al. 2015]
float solve_moments(in vec4 b, in float depth) {
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
float calculateMSMShadowTransparency(in vec4 b, in float depth, in float b_0) {
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
    //float f0 = overestimate;
    float f0 = 0.1;
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
float sample_moments(in vec4 moments, in float depth) {
    return solve_moments(bias_moments(moments), depth);
}

float sample_quantized_moments(in vec4 moments, in float depth) {
    return solve_moments(undo_quantization(moments), depth);
}

#endif