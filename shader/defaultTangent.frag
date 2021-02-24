#version 450
layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D texSamplerNormal;
//layout(set = 1, binding = 0) uniform samplerCube cubeSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat mat3 fragTangentFrame;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

void main() {
    vec3 lightDir = vec3(1, 1, 1);
    vec3 normal = vec3(texture(texSamplerNormal, fragTexCoord).rg * 2.0 - 1.0, 0);
    normal.z = 1-normal.x*normal.x - normal.y*normal.y;
    vec3 diffuseColor = texture(texSampler, fragTexCoord).rgb;
    float diffuse = dot(normal, lightDir);
    normal = fragTangentFrame * normal;
    outColor = vec4(diffuse * diffuseColor, 1.0);
    //outNormal = vec4(normal, 1);
    outNormal = vec4(normal, 1);
    //outNormal = vec4(texture(texSamplerNormal, fragTexCoord).rg, 1, 1);
    //outNormal = vec4(fragTangentFrame[2], 1);
    outColor = vec4(diffuseColor, 1);
}