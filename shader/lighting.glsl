
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
vec3 brdf_lambert(vec3 diffuseColor)
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

void calcDirLight(in vec3 albedo, in vec3 pbr, in vec3 viewDir, in vec3 normal, in DirectionalLight light, out vec3 specular, out vec3 diffuse) {
    
    float metalness = pbr.x;
    float alpha = pbr.y * pbr.y;

    float NdotL = max(dot(normal, light.dir), 0.0);
    float LdotV = dot(light.dir, viewDir);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float rcpLenLV = inversesqrt(2 + 2 * LdotV);
    float NdotH = (NdotL + NdotV) * rcpLenLV;
    float LdotH = rcpLenLV * LdotV + rcpLenLV;
    vec3 diffuseColor = (1- metalness) * albedo.xyz;
    diffuse = brdf_hammon_diffuse(NdotL, NdotV, NdotH, LdotV, LdotH, diffuseColor, alpha) * light.intensity * light.color * NdotL;
    
    vec3 F0 = vec3(0.04, 0.04, 0.04);
    vec3 specularColor = mix(F0 , albedo.xyz,metalness);
    specular = brdf_cook_torrance_specular(NdotL, NdotV, NdotH, LdotH, specularColor, alpha) * light.intensity * light.color;// * NdotL already canceled out
}