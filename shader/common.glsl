
vec4 fromLinear(vec4 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linearRGB.rgb * vec3(12.92);

    return vec4(mix(higher, lower, cutoff), linearRGB.a);
}

vec4 fromSRGB(vec4 sRGB)
{
    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.0031308 * 12.92));
    vec3 higher = pow(vec3(0.94786729) * (sRGB.rgb + vec3(0.055)), vec3(2.4));
    vec3 lower = sRGB.rgb / vec3(12.92);

    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

vec2 encodeOctahedronMapping(vec3 n) 
{
    n /= dot(abs(n), vec3(1));
    n.xy = mix(n.xy, (1.0 - abs(n.yx)) * fma(step(vec2(0.0), n.xy), vec2(2.0), vec2(-1.0)), step(n.z, 0.0));
    return n.xy * 0.5 + 0.5;
}

vec3 decodeOctahedronMapping(vec2 octahedron)
{
    octahedron = octahedron * 2.0 - 1.0;
    vec3 n = vec3(octahedron.xy, 1.0 - abs(octahedron.x) - abs(octahedron.y));
    n.xy = mix(n.xy, (1.0-abs(n.yx)) * fma(step(vec2(0.0), n.xy), vec2(2.0), vec2(-1.0)),step(n.z, 0));
    return normalize(n);
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

void orthonormalize(inout vec3 normal,inout vec3 tangent, inout vec3 bitangent) {
    //normal = normalize(normal);
    //tangent = normalize(tangent - dot(normal, tangent) * normal);
    //bitangent = normalize(bitangent - dot(normal, bitangent) * normal - dot(tangent, bitangent) * tangent);
    normal = normalize(normal);
    tangent = normalize(tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = normalize(cross(normal , tangent));
    // normal = normalize(cross(tangent, bitangent));
}

vec3 tangentSpaceNormal(in vec2 normalMapSample, in vec3 normal,in vec3 bitangent, in vec3 tangent)
{
    //Currently disabled because it produces bogus
//    vec2 convertedNormalMapSample = fma(normalMapSample,vec2(2.0), vec2(-1.0));
//    vec3 tangentNormal = vec3(convertedNormalMapSample.xy, sqrt(1.0-convertedNormalMapSample.x*convertedNormalMapSample.x - convertedNormalMapSample.y*convertedNormalMapSample.y));
//    vec3 tmpNormal = normal;
//    vec3 tmpTangent = tangent;
//    vec3 tmpBitangent = bitangent;
//    orthonormalize(tmpNormal, tmpTangent, tmpBitangent);
//	mat3 tangentFrame = mat3(tmpTangent , tmpBitangent , tmpNormal);
//    return tangentFrame * tangentNormal;


    return normalize(normal);
}