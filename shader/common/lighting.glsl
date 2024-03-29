#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL
#include "extracts.glsl"
#include "shaderStructs.glsl"

vec3 F_sphericalGaussian(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * exp2((-5.55473 * cosTheta -  6.98316) * cosTheta);
}

float pow5(float v) {
    float tmp = v *v;
    tmp = tmp * tmp;
    return tmp * v;
}

vec3 F_Schlick(float cosTheta, vec3 F0) {
    return  mix(F0 , vec3(1.0), pow5(1.0-cosTheta));
}
vec3 schlickLazanyi(float cosTheta, vec3 F0, vec3 a, float alpha) {
    return F_Schlick(cosTheta, F0) -
            a * cosTheta * pow(1.0 - cosTheta, alpha);
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

//Height correlated
//[E. Hammon, 2017] "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
float G2_smith(float NdotL, float NdotV, float alpha) {
//    float lambdaL = lambda_smith(NdotL, alpha);
//    float lambdaV = lambda_smith(NdotV, alpha);
//    return 1.0 / (1.0 + lambdaV + lambdaL);
    return 1.0f /  mix(2 * NdotL * NdotV, NdotL + NdotV, alpha);
}

//[E. Hammon, 2017] "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
vec3 brdf_hammon_diffuse(float NdotL, float NdotV,float NdotH, float VdotL, float VdotH, vec3 diffuseColor, float alpha)
{

    float facing = 0.5 + 0.5 * VdotL;

//    float energyBias = mix(0.0, 0.5, alpha);
//    float f90 = energyBias + 2.0 * VdotH * VdotH * alpha;
//    float fNdotL = (0.04 + (f90 - 0.04) * exp2((-5.55473 * NdotL - 6.98316) *NdotL));
//    float fNdotV = (0.04 + (f90 - 0.04) * exp2((-5.55473 * NdotV - 6.98316) *NdotV));
    //float surfaceRough =  mix( 0.0, (facing * (0.9 - 0.4 * facing) * (0.5 + NdotH) / NdotH), NdotH <= 0.0 );
    //float surfaceSmooth = 1.05 * ((1.0 - fNdotL) * (1.0 - fNdotV));

    float surfaceRough =  facing * (0.9 - 0.4 * facing) * (0.5 + NdotH) / max(NdotH, 1e-8f);
    float surfaceSmooth = 1.05 * (1. - 0.04) * (1.0 - pow5(1.f - NdotL)) * (1.0 - pow5(1.f - NdotV));
    float single = 1. / 3.1415926 * mix(surfaceSmooth, surfaceRough, alpha);
    float multi = 0.1159 * alpha;
    //float multi = 0.3641 * alpha;

    return diffuseColor.xyz * (single + diffuseColor.xyz * multi);// * sign(NdotV)* sign(NdotL);
}

//[Stephen McAuley, 2018] "A Journey Through Implementing BRDFs & Area Lights"
// Who adapted "Material Advances in Call of Duty: WWII"
float multi_scattering_diffuse_brdf(float NdotL, float NdotV, float NdotH, float LdotH, float alpha) {
    float g = min(1.0, max(0.0, 0.18455 * log(2.0/ ((alpha * alpha)) - 1.0 )));

    float f0 = LdotH + pow5(1.0f - LdotH);
    float f1 = (1.0 - 0.75 * pow5(1.0 - NdotL)) * (1.0 - 0.75 * pow5(1.0 - NdotV));
    float t = min(1.0, max(0.0, 2.2 * g - 0.5));
    float fd = f0 + (f1 - f0) * t;
    float fb = ((34.5 * g - 59.0) * g + 24.5) * LdotH * exp2(-max(73.2 * g - 21.2, 8.9) * sqrt(NdotH));
    
    return fd + fb;
}

float brdf_lambert()
{
    const float single = 1.f / 3.1415926f;

    return  single;
}

vec3 brdf_cook_torrance_specular(float NdotL, float NdotV, float NdotH, float LdotH, vec3 specularColor, float alpha) {
    float NDF = NDF_GGX(NdotH, alpha);
    
    float G2 = G2_smith(NdotL, NdotV, alpha);

    vec3 F = F_sphericalGaussian(LdotH, specularColor);

    return max((NDF * G2 * 0.5)* F, vec3(0.f)); // /  4.0 * NdotV * NdotL Canceled out with G2
}
//
//float brdf_cook_torrance_specular(float NdotL, float NdotV, float NdotH, float LdotH, vec3 specularColor, float alpha) {
//    float NDF = NDF_GGX(NdotH, alpha);
//    
//    float G2 = G2_smith(NdotL, NdotV, alpha);
//
//    //vec3 F = F_sphericalGaussian(LdotH, specularColor);
//
//    return max(0, NDF * G2 * 0.5); // /  4.0 * NdotV * NdotL Canceled out with G2
//}

void calcDirLight(in vec3 albedo, in float metalness, in float roughness, in vec3 viewDir, in vec3 normal, in DirectionalLight light, inout vec4 specular, inout vec4 diffuse) {
    
    float alpha = roughness * roughness;

    float NdotL = max(dot(normal, light.dir), 0.0);
    float LdotV = dot(light.dir, viewDir);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float rcpLenLV = inversesqrt(2 + 2 * LdotV);
    float NdotH = (NdotL + NdotV) * rcpLenLV;
    float LdotH = rcpLenLV * LdotV + rcpLenLV;


    //diffuse.xyz += brdf_hammon_diffuse(NdotL, NdotV, NdotH, LdotV, LdotH, albedo.xyz, alpha) *( (1- metalness) * light.intensity * NdotL) * light.color ;

    vec3 specularColor = mix( vec3(0.04), albedo.xyz, metalness);
    vec3 F = F_Schlick(NdotV, specularColor);

    //diffuse.xyz += multi_scattering_diffuse_brdf(NdotL, NdotV, NdotH, LdotH, alpha)*( (1- metalness) * light.intensity * NdotL * albedo.xyz ) * (vec3(1.f) -F) * light.color ; 
    //diffuse.xyz += brdf_lambert()*( (1- metalness) * light.intensity * albedo.xyz * NdotL) * light.color;
    //TODO Energy Compensation for multiple Scattering
    //Consider Energy Compensation from: Revisiting Physically Based Shading at Imageworks
	//Sony Pictures Imageworks�s Lighting Model Integration Report
	//A Multiple-scattering Microfacet Model for Real-time Image-based Lighting
	//https://blog.selfshadow.com/2018/06/04/multi-faceted-part-2/
	//https://blog.selfshadow.com/2018/08/05/multi-faceted-part-3/
	// https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance
	// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf
    //Fdez-Ag�era, A Multiple-Scattering Microfacet Model for Real-Time Image Based Lighting.

    specular.xyz += (brdf_cook_torrance_specular(NdotL, NdotV, NdotH, LdotH, specularColor, alpha) * light.intensity * NdotL) * F * light.color ;
}



void diffuse_light(in LightData light, in ShadingData shadingData, inout vec3 diffuse)
{
    float NdotL = max(dot(shadingData.shadingNormal, light.dir), 0.0);
    if(NdotL <= 0)
        return;

    float LdotV = max(dot(light.dir, shadingData.outLightDir), 0.0);
    float NdotV = max(dot(shadingData.shadingNormal, shadingData.outLightDir), 0.0);
    float rcpLenLV = inversesqrt(2 + 2 * LdotV);
    float NdotH = (NdotL + NdotV) * rcpLenLV;
    float LdotH = rcpLenLV * LdotV + rcpLenLV;
    
    vec3 specularColor = mix( vec3(0.04), shadingData.albedo.xyz, shadingData.metalness);
    vec3 F = F_Schlick(NdotV, specularColor);
    
    //if( shadingData.metalness < 1.f)
    //    diffuse.xyz += multi_scattering_diffuse_brdf(NdotL, NdotV, NdotH, LdotH, shadingData.alpha)*( (1- shadingData.metalness) * light.intensity * NdotL * shadingData.albedo.xyz ) * (vec3(1.f) -F) * light.color ; 
    //diffuse += multi_scattering_diffuse_brdf(NdotL, NdotV, NdotH, LdotH, shadingData.alpha)*( (1- shadingData.metalness) * light.intensity  * NdotL)* (vec3(1.f) -F) * shadingData.albedo.xyz * light.color;
    //diffuse += brdf_lambert()*( (1- shadingData.metalness) * light.intensity * NdotL)* (vec3(1.f) -F) * shadingData.albedo.xyz  * light.color;
    //diffuse += brdf_lambert()*(light.intensity * NdotL) * shadingData.albedo.xyz * light.color;
    //diffuse.xyz += (21.f/ (20.0f * 3.1415926f)) * (1- specularColor) * shadingData.albedo.xyz * (1- pow(1- NdotL, 5))* (1- pow(1- NdotV, 5))* light.intensity* light.color;
    diffuse.xyz += brdf_hammon_diffuse(NdotL, NdotV, NdotH, LdotV, dot(normalize( shadingData.outLightDir + light.dir ), shadingData.outLightDir ), shadingData.albedo.xyz, sqrt(shadingData.alpha) )*( (1- shadingData.metalness) * light.intensity * NdotL) * light.color ; 

}

void calc_light(in LightData light, in ShadingData shadingData, inout vec3 diffuse, inout vec3 specular)
{
//    diffuse_light(light, shadingData, diffuse);
//    return;


    float NdotL = max(dot(shadingData.shadingNormal, light.dir), 0.0);
    if(NdotL <= 0)
        return;
        
    float LdotV = max(dot(light.dir, shadingData.outLightDir), 0.0);
    float NdotV = max(dot(shadingData.shadingNormal, shadingData.outLightDir), 0.0);
    float rcpLenLV = inversesqrt(2 + 2 * LdotV);
    float NdotH = (NdotL + NdotV) * rcpLenLV;
    float LdotH = rcpLenLV * LdotV + rcpLenLV;
    
    vec3 specularColor = mix( vec3(0.04), shadingData.albedo.xyz, shadingData.metalness);
    //vec3 F = F_Schlick(NdotV, specularColor);

    //if( shadingData.metalness < 1.f)
        //diffuse.xyz += (21.f/ (20.0f * 3.1415926f)) * (1- specularColor) * shadingData.albedo.xyz * (1- pow(1- NdotL, 5))* (1- pow(1- NdotV, 5))* light.intensity* light.color;;
        //diffuse.xyz += multi_scattering_diffuse_brdf(NdotL, NdotV, NdotH, LdotH, shadingData.alpha)*( (1- shadingData.metalness) * light.intensity * NdotL * shadingData.albedo.xyz ) * (vec3(1.f) -F) * light.color ; 
    diffuse.xyz += brdf_hammon_diffuse(NdotL, NdotV, NdotH, LdotV, dot(normalize( shadingData.outLightDir + light.dir ), shadingData.outLightDir ), shadingData.albedo.xyz, sqrt(shadingData.alpha) )*( (1- shadingData.metalness) * light.intensity * NdotL) * light.color ; 
    //diffuse.xyz += brdf_lambert()* light.intensity * NdotL *(shadingData.albedo.xyz * light.color) ; 
    
    specular.xyz += (brdf_cook_torrance_specular(NdotL, NdotV, NdotH, LdotH, specularColor, shadingData.alpha) * light.intensity * NdotL) * light.color ;
    //specular.xyz += (brdf_cook_torrance_specular(NdotL, NdotV, NdotH, LdotH, specularColor, shadingData.alpha) * light.intensity * NdotL) * light.color ;
}


#endif