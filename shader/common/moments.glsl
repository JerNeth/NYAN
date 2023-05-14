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

//void calculateMoments4Quant(infloat depth, out vec4 moments) {
//   float depthSquared = depth*depth;
//
//    moments = vec4(depth, depthSquared, depthSquared*depth, depthSquared*depthSquared);
//    quantMoments(moments);
//}
//void calculateMoments4QuantSNorm(infloat depth, out vec4 moments) {
//    //remap [0;1] -> [-1;1] [Peters et al. 2017]
//    //Proved to be more robust when stored in 16 bits
//   float depthSNorm = (depth*2.0f)-1.0f;
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
    //floatf0 = overestimate;
    float f0 = 0.1;
    float f1 = z.y < z.x?1.0f : 0.0f;
    float f2 = z.z < z.x?1.0f : 0.0f;
    float f01 = (f1-f0)/(z.y-z.x);
    float f12 = (f2-f1)/(z.z-z.y);
    float f012 = (f12-f01)/(z.z -z.x);
    polynomial.x =f01-f012*z.y;
    polynomial.z =f012;
    polynomial.y = polynomial.x -f012*z.x;
    polynomial.x =f0-polynomial.x*z.x;
    float reconstructedOpticalDepth = polynomial.x + dot(b.xy, polynomial.yz);
    float temp = clamp(exp(-b_0*reconstructedOpticalDepth),0.0f, 1.0f);
    return temp;
}

float sample_quantized_moments(in vec4 moments, in float depth) {
    return solve_moments(undo_quantization(moments), depth);
}
float Hausdorff4MSM (in vec4 b ,in float Depth ) {
    vec3 z ;
    z.x = Depth;
    // Cholesky d e c om p o si ti o n
    float L21D11=fma(-b[ 0 ] , b[ 1 ] , b[ 2 ] );
    float D11=fma(-b[ 0 ] , b[ 0 ] , b[ 1 ] );
    float SquaredDepthVariance=fma(-b[ 1 ] , b[ 1 ] , b[ 3 ] ) ;
    float D22D11=dot ( vec2 ( SquaredDepthVariance ,-L21D11 ) ,
    vec2 ( D11 , L21D11 ) ) ;
    float InvD11=1.0f/D11 ;
    float L21=L21D11*InvD11 ;
    float D22=D22D11*InvD11 ;
    float InvD22=1.0f/D22 ;
    // S ol u ti o n of a l i n e a r system with Cholesky
    vec3 c=vec3 ( 1.0f , z [ 0 ] , z [ 0 ] * z [ 0 ] ) ;
    c[1]-=b.x ;
    c[2]-=b.y+L21*c [ 1 ] ;
    c[1]*= InvD11 ;
    c[2]*= InvD22 ;
    c[1]-=L21*c [ 2 ] ;
    c[0]-=dot ( c.yz , b.xy ) ;
    // Qu ad r a tic e q u a ti o n s ol v e d with the q u a d r a ti cf o rmul a
    float InvC2=1.0f/c [ 2 ] ;
    float p=c [ 1 ] * InvC2 ;
    float q=c [ 0 ] * InvC2 ;
    float D=(p*p *0.25f )-q ;
    float r= sqrt( D ) ;
    z[1] =-p * 0.5f-r ;
    z[2] =-p * 0.5f+r ;
    // The s o l u t i o n u s e sf o u r p oi n t s of supp o r t
    if( z[1] < -1.0f || z [2] > 1.0f ) {
        float zFree=((b[0] -b[ 2 ] ) *z [0]+ b[3] -b[ 1 ] ) / ( z [ 0]+ b[2] -b[0] -b[ 1 ] * z [0] ) ;
        float w1Factor=(z[0] > zFree ) ? 1.0f : 0.0f ;
        // C on s t ruc t an i n t e r p o l a t i o n p ol yn omi al
        vec2 Normalizers=vec2 (
             w1Factor / ( ( zFree-z[0] ) *fma( zFree , zFree , -1.0f ) ) ,
            0.5f / ( ( zFree+1.0f ) *( z[0] + 1.0f ) ) ) ;
        vec4 Polynomial ;
        Polynomial[ 0]=fma( zFree , Normalizers.y , Normalizers.x ) ;
        Polynomial[ 1]= Normalizers.x-Normalizers.y ;
        // M ul ti pl y by ( z-z [ 0 ] )
        Polynomial[ 2]= Polynomial [ 1 ] ;
        Polynomial[ 1]=fma( Polynomial [ 1] , - z [ 0 ] , Polynomial [ 0 ] ) ;
        Polynomial[0]*=-z [ 0 ] ;
        // M ul ti pl y by ( z-1)
        Polynomial[ 3]= Polynomial [ 2 ] ;
        Polynomial.yz = Polynomial.xy-Polynomial.yz ;
        Polynomial[0] *= -1.0f ;
        // Ev alu a te the shadow i n t e n s i t y
        return clamp( dot ( Polynomial , vec4 ( 1.0f , b . xyz ) ) , 0.f, 1.f) ;
    }
    // The s o l u t i o n u s e s t h r e e p oi n t s of supp o r t
    else {
        vec4 Switch=
        ( z [2] <z [ 0 ] ) ? vec4 ( z [ 1 ] , z [ 0 ] , 1.0f , 1.0f ) : (
        ( z [1] <z [ 0 ] ) ? vec4 ( z [ 0 ] , z [ 1 ] , 0.0f , 1.0f ) :
        vec4 ( 0.0f , 0.0f , 0.0f , 0.0f ) ) ;
        float Quotient=(Switch [ 0 ] * z[2] -b[ 0 ] * ( Switch [ 0]+ z [ 2 ] )+b[ 1 ] )
        / ( ( z [2] - Switch [ 1 ] ) *( z[0] -z [ 1 ] ) ) ;
        return clamp( Switch [2]+ Switch [ 3 ] * Quotient, 0.f, 1.f) ;
    }
}
vec3 SolveCubic ( vec4 Coefficient ) {
    Coefficient.xyz/=Coefficient.w ;
    Coefficient.yz /=3.0f ;
    vec3 Delta=vec3 (
    fma(-Coefficient.z , Coefficient.z , Coefficient.y ) ,
    fma(-Coefficient.y , Coefficient.z , Coefficient.x ) ,
    dot ( vec2 ( Coefficient . z,-Coefficient.y ) , Coefficient.xy ) ) ;
    float Discriminant=dot ( vec2 ( 4.0f*Delta . x,-Delta.y ) , Delta.zy ) ;
    vec2 Depressed=vec2 (
    fma( -2.0f* Coefficient.z , Delta.x , Delta.y ) ,
    Delta . x ) ;
    float Theta=atan(  -Depressed.x, sqrt( Discriminant ) ) / 3.0f ;
    vec2 CubicRoot  = vec2(cos(Theta), sin(Theta));
    vec3 Root=vec3 (
    CubicRoot.x ,
    dot ( vec2 ( -0.5f , -0.5f* sqrt( 3.0f ) ) , CubicRoot ) ,
    dot ( vec2 ( -0.5f ,  0.5f* sqrt( 3.0f ) ) , CubicRoot ) ) ;
    return fma(vec3( 2.0f* sqrt(-Depressed.y )) , Root ,vec3(-Coefficient.z )) ;
}
float Hamburger6MSM ( float b [6] , float Depth , float Overestimation ) {
    const float MomentBias = 3.0e-7f;
    b[0] = mix(b[0], 0.f, MomentBias);
    b[1] = mix(b[1], 0.375f, MomentBias);
    b[2] = mix(b[2], 0.f, MomentBias);
    b[3] = mix(b[3], 0.375f, MomentBias);
    b[4] = mix(b[4], 0.f, MomentBias);
    b[5] = mix(b[5], 0.375f, MomentBias);
    vec4 z ;
    z [0]= Depth ;
    // Cholesky d e c om p o si ti o n
    float InvD11=1.0f/fma(-b [ 0 ] , b [ 0 ] , b [ 1 ] ) ;
    float L21D11=fma(-b [ 0 ] , b [ 1 ] , b [ 2 ] ) ;
    float L21=L21D11*InvD11 ;
    float D22=fma(-L21D11 , L21 , fma(-b [ 1 ] , b [ 1 ] , b [ 3 ] ) ) ;
    float L31D11=fma(-b[ 0 ] , b [ 2 ] , b [ 3 ] ) ;
    float L31=L31D11*InvD11 ;
    float InvD22=1.0f/D22 ;
    float L32D22=fma(-L21D11 , L31 , fma(-b [ 1 ] , b [ 2 ] , b [ 4 ] ) ) ;
    float L32=L32D22*InvD22 ;
    float D33=fma(-b [ 2 ] , b [ 2 ] , b [ 5 ] )-dot ( vec2 ( L31D11 , L32D22 ) ,
    vec2 ( L31 , L32 ) ) ;
    float InvD33=1.0f/D33 ;
    // S ol u ti o n o f a l i n e a r system with Cholesky
    vec4 c ;
    c [ 0] = 1.0f ;
    c [ 1]= z [ 0 ] ;
    c [ 2]= c [ 1 ] * z [ 0 ] ;
    c [ 3]= c [ 2 ] * z [ 0 ] ;
    c[1]-=b [ 0 ] ;
    c[2]-=fma( L21 , c [ 1 ] , b [ 1 ] ) ;
    c[3]-=b [ 2]+ dot ( vec2 ( L31 , L32 ) , c . yz ) ;
    c.yzw*=vec3 ( InvD11 , InvD22 , InvD33 ) ;
    c[2]-=L32*c [ 3 ] ;
    c[1]-=dot ( vec2 ( L21 , L31 ) , c . zw ) ;
    c[0]-=dot ( vec3 ( b [ 0 ] , b [ 1 ] , b [ 2 ] ) , c . yzw ) ;
    // S ol v e the c u bi c e q u a ti o n
    z . yzw=SolveCubic( c ) ;
    // Determine the c o n t r i b u t i o n t o the end r e s u l t
    vec4 WeightFactor ;
    WeightFactor[ 0]= Overestimation ;
    WeightFactor.yzw=all(greaterThan(z.yzw,z.xxx )) ? vec3 ( 0.0f , 0.0f , 0.0f ) :
    vec3 ( 1.0f ,1.0f , 1.0f ) ;
    // C on s t ruc t an i n t e r p o l a t i o n p ol yn omi al
    float f0=WeightFactor [ 0 ] ;
    float f1=WeightFactor [ 1 ] ;
    float f2=WeightFactor [ 2 ] ;
    float f3=WeightFactor [ 3 ] ;
    float f01=(f1-f0 ) / ( z [1] -z [ 0 ] ) ;
    float f12=(f2-f1 ) / ( z [2] -z [ 1 ] ) ;
    float f23=(f3-f2 ) / ( z [3] -z [ 2 ] ) ;
    float f012=(f12-f01 ) / ( z [2] -z [ 0 ] ) ;
    float f123=(f23-f12 ) / ( z [3] -z [ 1 ] ) ;
    float f0123=(f123-f012 ) / ( z [3] -z [ 0 ] ) ;
    vec4 Polynomial ;
    // f 0 1 2+f 0 1 2 3 *( z-z2 )
    Polynomial [ 0]=fma(-f0123 , z [ 2 ] , f012 ) ;
    Polynomial [ 1]= f0123 ;
    // *( z-z1 ) +f 0 1
    Polynomial [ 2]= Polynomial [ 1 ] ;
    Polynomial [ 1]=fma( Polynomial [ 1] , -z[ 1 ] , Polynomial [ 0 ] ) ;
    Polynomial [ 0]=fma( Polynomial [ 0] , -z[ 1 ] , f01 ) ;
    // *( z-z0 ) +f 0
    Polynomial [ 3]= Polynomial [ 2 ] ;
    Polynomial [ 2]=fma( Polynomial [ 2] , -z[ 0 ] , Polynomial [ 1 ] ) ;
    Polynomial [ 1]=fma( Polynomial [ 1] , -z[ 0 ] , Polynomial [ 0 ] ) ;
    Polynomial [ 0]=fma( Polynomial [ 0] , -z[ 0 ] , f0 ) ;
    // Ev alu a te the shadow i n t e n s i t y
    return clamp(  dot ( Polynomial , vec4 ( 1.0f , b[0] , b[1] , b[2]) ), 0.f, 1.f) ;
}

float sample_moments(in vec4 moments, in float depth) {
    return solve_moments(bias_moments(moments), depth);
}
#endif